#include <stdio.h>
#include <string.h>    
#include <stdlib.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>    
#include <pthread.h> 
 
//the thread function
void *connection_handler(void *);
int alert(char *msg);

// Estructura que sirve para pasar los parametros que los hilos se deben llevar a la funcion donde los hilos reciben la informacion de los clientes
typedef struct hilo 									
{														
	int fd; 							// File descriptor
	char *msg;							// Cadena de caracteres donde se almacenan los mensajes que recibe el servidor
	char *output; 						// Cadena de caracteres donde se almacena el Nombre del archivo de salida
	pthread_mutex_t mutex; 				// Mutex que evita que varios hilos escriban sobre el archivo de salida al mismo tiempo
}hilo;

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c, port;
    struct sockaddr_in server , client;
 	char *output = (char *) malloc(512*sizeof(char));

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

    //Receive a message from client
    while( (read_size = read(hi->fd , hi->msg , 2048)) > 0 )
    {
        //Send the message back to client
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
	 	}
	 	else
	 	{
	 		fprintf(fp, "%s",hi->msg);
			printf("%s\n", hi->msg);
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
    if (strcmp(msg,"Communication Offline\n") == 0)
    	return 1;

    if (strcmp(msg,"Communication error\n") == 0)
        return 1;

    if (strcmp(msg,"Low Cash alert\n") == 0)
        return 1;
    
    if (strcmp(msg,"Running Out of notes in cassette\n") == 0)
        return 1;

    if (strcmp(msg,"empty\n") == 0)
        return 1;

    if (strcmp(msg,"Service mode entered\n") == 0)
        return 1;

    if (strcmp(msg,"Service mode left\n") == 0)
      	return 1;

    if (strcmp(msg,"device did not answer as expected\n") == 0)
        return 1;

    if (strcmp(msg,"The protocol was cancelled\n") == 0)
        return 1;

    if (strcmp(msg,"Low Paper warning\n") == 0)
        return 1;
    
    if (strcmp(msg,"Printer Error\n") == 0)
        return 1;

    if (strcmp(msg,"Paper-out condition\n") == 0)
        return 1;

    return 0;
}