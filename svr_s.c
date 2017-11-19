#include <stdio.h>
#include <string.h>    
#include <stdlib.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>    
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

#include "header.h"


int input_timeout (int filedes, unsigned int seconds){      // Se crea una función que recibe el file descriptor del socket y una cantidad 
                                                            // de tiempo en segundos    
    fd_set set;                                             
    struct timeval timeout;


    FD_ZERO (&set);                                         // Se inicializa el fd del socket
    FD_SET (filedes, &set);

    timeout.tv_sec = seconds;                               // Inicializa la estructura de tiempo
    timeout.tv_usec = 0;

    return (select (FD_SETSIZE,&set, NULL, NULL,&timeout)); // Devuelve 1 si está activo el canal; 0 si no está activo y -1 en caso de error
}

int mail_alert(char *message){
    char* command = (char *) malloc(1000*sizeof(char));
    char* mailto;
    if((mailto = getenv("MAILTO")) != NULL){
        strcpy(command, "echo \"ALERT: ");
        strncat(command, message, strlen(message)-1);
        strcat(command, "\" | mail -s \"REDBANC ALERT\" ");
        strcat(command, mailto);
        system(command);
        printf("MAIL COMMAND: %s\n", command);
    }
    else{
        printf("Please set the $MAILTO variable in your environment to the account where you want to recieve these messages\n");
    }
    return 0;
}



int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c, port;								// port: puerto por donde escuchara el servidor
    struct sockaddr_in server , client;
 	char *output = (char *) malloc(512*sizeof(char));						// Cadena de caracteres donde se almacenara el nombre del archivo de salida	

 	// Condicionales que permiten que el comando funcione con la sintaxis deseada
    if (argc != 5)
    {
    	printf("Wrong Sintaxis.\nPlease use: svr_s -l <puerto_svr_s> -b <archivo_bitácora>\n");
    	exit(-1);
    }
    if ((strcmp(argv[1],"-l") == 0) && (strcmp(argv[3],"-b") == 0)) 			// Caso svr_s -l <puerto_svr_s> -b <archivo_bitácora>
    {
    	port = atoi(argv[2]);
    	strcpy(output, argv[4]);
    }
    else if((strcmp(argv[1],"-b") == 0) && (strcmp(argv[3],"-l") == 0)) 		// Caso svr_s  -b <archivo_bitácora> -l <puerto_svr_s>
    {
    	port = atoi(argv[4]);
    	strcpy(output, argv[2]);
    }
    else
    {
    	printf("Wrong Sintaxis.\nPlease use: svr_s -l <puerto_svr_s> -b <archivo_bitácora>\n");
    	exit(-1);
    }
 
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
        printf("Could not create socket");
    
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET; 					// IPv4
    server.sin_addr.s_addr = INADDR_ANY;			// Recibe clientes con cualquier direccion IP
    server.sin_port = htons( port ); 				// Oye por el puerto indicado por el usuario 
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }

    puts("The SVR module has been initialized");

    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming  ATM connections...");
    c = sizeof(struct sockaddr_in);

    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        pthread_t sniffer_thread;

        // Se inicializan parametros que se llevara el hilo a la funcion donde estaran recibiendo informacion del cliente
        hilo *h = (hilo*) malloc(sizeof(hilo));
        h->fd = client_sock;
        h->msg = (char*) malloc(2048*sizeof(char));
        h->output = (char*) malloc(512*sizeof(char));
        strcpy(h->output, output);
        pthread_mutex_init(&h->mutex, NULL);;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) h) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        pthread_detach(sniffer_thread);
        puts("ATM Connected\n");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    return 0;
}


// Funcion que mantiene la conexion con los clientes, escribiendo los mensajes entrantes en un archivo. 
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor

    hilo *hi = (hilo*) socket_desc; 					// CAST
    int read_size;
	FILE* fp;
    int optval = 1;
    socklen_t optlen = sizeof(optval);
 
	if(setsockopt(hi->fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0)
	{
	    perror("setsockopt()");
	    close(hi->fd);
	    exit(-1);
	}

    //Recibe mensaje del cliente
    while( (read_size = read(hi->fd , hi->msg , 2048)) > 0 )
    {
        int aux = 0;

        // Verifico si el mensaje entrante posee informacion de interes para generar una alerta
        aux = alert(hi->msg);


        // Comienzo de region critica, solo un hilo pasa a la vez
		pthread_mutex_lock(&hi->mutex);


		if(!(fp = fopen(hi->output,"a")))
		{
			printf("Error, Permission denied. Non generated report\n");
			exit(-1);
		}   
	 	if (alert(hi->msg))
	 	{
	 		printf("ALERT: %s\n", hi->msg);
	 	 	fprintf(fp, "ALERT: %s",hi->msg);
            mail_alert(hi->msg);
	 	}
	 	else
	 	{

            fprintf(fp, "%s",hi->msg);
            printf("%s\n", hi->msg);
            
            while (!input_timeout(hi->fd,10)){             // Se esperan 5 minutos para el envío de data.

                fprintf (stderr, "\nNo data sent \n");      // Se informa que no se ha enviado data, pero mantiene activa la conexión
                fprintf(fp, "ALERT: No data sent");         // Envía la alerta al archivo
            }
	 		    

            
		}
		fclose(fp);
  		pthread_mutex_unlock(&hi->mutex); 
  		// Fin region critica

        memset(hi->msg, '\0', strlen(hi->msg));
        fflush(stdout);
    }


     
    if(read_size == 0)
    {
        puts("ATM Disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
        perror("recv failed");
             
    //Free the socket pointer
    free(socket_desc);
    pthread_exit(NULL);
    return 0;
}


//Funcion que compara cadenas de caracteres y retorna 1 si hay coincidencia (hay alerta) y cero de  lo contrario
int alert(char *msg)
{

    int code1 = msg[12] - '0';
    int code2 = msg[13] - '0';

    if (code1 == 0)
    {
        for (int j = 1; j < 10; j++)
            if(code2 == j)
                return 1;

    }else if (code1 == 1)
    {
        for (int j = 0; j < 3; j++)
            if(code2 == j)
                return 1;
    }
    return 0;
}