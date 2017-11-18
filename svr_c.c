#include <stdio.h> 
#include <string.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

#include "header.h"


int input_timeout (int filedes, unsigned int seconds){

	fd_set set;
	struct timeval timeout;

	/* Initialize the file descriptor set. */
	FD_ZERO (&set);
	FD_SET (filedes, &set);

	/* Initialize the timeout data structure. */
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;

	/* select returns 0 if timeout, 1 if input available, -1 if error. */
	return (select (FD_SETSIZE,&set, NULL, NULL,&timeout));
}


int main(int argc , char *argv[])
{
    int sock, port, local_port;								// sock: file descriptor del socket. port: puerto del servidor. local_port: puerto del cliente.
    struct sockaddr_in server;								// server almacenara los parametros del servidor
    struct sockaddr_in client;								// Estructura usada para almacenar el puerto del cliente
    char *message = (char*) malloc(2048*sizeof(char)); 		// Cadena de caracteres que almacena los mensajes que se transmitiran al servidor
    char *ip_srv = (char *) malloc(512*sizeof(char)); 		// Cadena de caracteres donde se almacena la direccion ip del servidor indicado por el usuario
    int flag = 0, flag2=0;			 						// flag: indica cuando el servidor esta caido. flag2: 1 cuando se indica el servidor por nombre de dominio
    int flag3 = 0;											// flag3: indica cuando se pasa el puerto del cliente como parametro
    struct hostent *servidor; 								// Estructura unicamente usada si se indica el servidor por nombre de dominio

    // Cadenas de caracteres auxiliares que son usadas cuando se cae la conexion con el servidor para encolar los mensajes que se tratan de enviar sin conexion
    char *pp = (char *) malloc(2048*sizeof(char));
    char *sp = (char *) malloc(2048*sizeof(char));

    // Selector que permite que el programa funcione con la sintaxis deseada svr_c -d <nombre_módulo_central> -p <puerto_svr_s> [-l <puerto_local>]
    switch(argc) 									
	{
		case 5:	
				if ((strcmp(argv[1],"-d") == 0) && (strcmp(argv[3],"-p") == 0))
			    {
			    	strcpy(ip_srv, argv[2]);
			    	
			    	if (ip_srv[0] < 48 || ip_srv[0] > 57) 						// En caso de que se indique el servidor por nombre de dominio
			    	{
			    		servidor = gethostbyname(argv[2]); 
			    		if (!servidor )
			    		{
			    			puts("Invalid server addres");
			    			return -1;
			    		}
			       		flag2 = 1;
			       	}
			    	
			    	port = atoi(argv[4]);
			    }
			    else if((strcmp(argv[1],"-p") == 0) && (strcmp(argv[3],"-d") == 0))
			    {
			    	strcpy(ip_srv, argv[4]); 

			    	if (ip_srv[0] < 48 || ip_srv[0] > 57) 						// En caso de que se indique el servidor por nombre de dominio
			    	{
			    		servidor = gethostbyname(argv[4]);
			    		if (!servidor )
			    		{
			    			puts("Invalid server addres");
			    			return -1;
			    		}
			       		flag2 = 1;
			        }
			    	
			    	port = atoi(argv[2]);
			    }
			    break;

		case 7:	
			    if ((strcmp(argv[1],"-d") == 0) && (strcmp(argv[3],"-p") == 0) && (strcmp(argv[5],"-l") == 0))
			    {
			    	strcpy(ip_srv, argv[2]);

			    	if (ip_srv[0] < 48 || ip_srv[0] > 57) 			// En caso de que se indique el servidor por nombre de dominio
			    	{
			    		servidor = gethostbyname(argv[2]); 
			       		if (!servidor )
			    		{
			    			puts("Invalid server addres");
			    			return -1;
			    		}
			       		flag2 = 1;
			        }
			    	
			    	port = atoi(argv[4]);
			    	local_port = atoi(argv[6]);
			    }
			    else if((strcmp(argv[1],"-d") == 0) && (strcmp(argv[3],"-l") == 0) && (strcmp(argv[5],"-p") == 0))
			    {
			    	strcpy(ip_srv, argv[2]);

			    	if (ip_srv[0] < 48 || ip_srv[0] > 57) 		// En caso de que se indique el servidor por nombre de dominio
			    	{
			    		servidor = gethostbyname(argv[2]); 
			       		if (!servidor )
			    		{
			    			puts("Invalid server addres");
			    			return -1;
			    		}
			       		flag2 = 1;
			        } 

			    	port = atoi(argv[6]);
			    	local_port = atoi(argv[4]);
			    }
			    else if((strcmp(argv[1],"-p") == 0) && (strcmp(argv[3],"-d") == 0) && (strcmp(argv[5],"-l") == 0))
			    {
			    	strcpy(ip_srv, argv[4]); 

			    	if (ip_srv[0] < 48 || ip_srv[0] > 57)  		// En caso de que se indique el servidor por nombre de dominio
			    	{
			    		servidor = gethostbyname(argv[4]); 
			       		if (!servidor )
			    		{
			    			puts("Invalid server addres");
			    			return -1;
			    		}
			       		flag2 = 1;
			        }
			    	
			    	port = atoi(argv[2]);
			    	local_port = atoi(argv[6]);
			    }
			    else if((strcmp(argv[1],"-p") == 0) && (strcmp(argv[3],"-l") == 0) && (strcmp(argv[5],"-d") == 0))
			    {
			    	strcpy(ip_srv, argv[6]);

			    	if (ip_srv[0] < 48 || ip_srv[0] > 57) 			// En caso de que se indique el servidor por nombre de dominio
			    	{
			    		servidor = gethostbyname(argv[6]); 
			    		if (!servidor )
			    		{
			    			puts("Invalid server addres");
			    			return -1;
			    		}
			    		flag2 = 1;
			    	}			    		  
			    	
			    	port = atoi(argv[2]);
			    	local_port = atoi(argv[4]);
			    }
			    else if((strcmp(argv[1],"-l") == 0) && (strcmp(argv[3],"-d") == 0) && (strcmp(argv[5],"-p") == 0))
			    {
			    	strcpy(ip_srv, argv[4]);

			    	if (ip_srv[0] < 48 || ip_srv[0] > 57) 			// En caso de que se indique el servidor por nombre de dominio
			    	{
			    		servidor = gethostbyname(argv[4]);
			    		if (!servidor )
			    		{
			    			puts("Invalid server addres");
			    			return -1;
			    		}
			    		flag2 = 1;
			    	} 
			    	
			    	port = atoi(argv[6]);
			    	local_port = atoi(argv[2]);
			    }
			    else if((strcmp(argv[1],"-l") == 0) && (strcmp(argv[3],"-p") == 0) && (strcmp(argv[5],"-d") == 0))
			    {
			    	strcpy(ip_srv, argv[6]);
			    	
			    	if (ip_srv[0] < 48 || ip_srv[0] > 57)  				// En caso de que se indique el servidor por nombre de dominio
			    	{	
			    		servidor = gethostbyname(argv[6]); 
			    		if (!servidor )
			    		{
			    			puts("Invalid server addres");
			    			return -1;
			    		}
			    		flag2 = 1;
			    	}
			    		  
			    	port = atoi(argv[4]);
			    	local_port = atoi(argv[2]);
			    }
			    flag3=1;
				break;

		default:
				printf("Wrong Sintaxis.\nsvr_c -d <nombre_módulo_central> -p <puerto_svr_s> [-l <puerto_local>]\n");
    			exit(-1);
	}
	while(1)
	{
	    //Create socket
	    sock = socket(AF_INET , SOCK_STREAM , 0);

	    if (sock == -1)
	    {
	        printf("Could not create socket");
	        continue;
	    }

	    if (flag2) 			// Si el servidor se indico por nombre de dominio 
	    {
	    	bzero((char *)&server, sizeof((char *)&server));
	    	bcopy((char *)servidor->h_addr, (char *)&server.sin_addr.s_addr, sizeof(servidor->h_length));
	    }
	    else
	       	server.sin_addr.s_addr = inet_addr(ip_srv); 			// Se indica la direccion del servidor indicado por usuario
	    
	    server.sin_family = AF_INET;							// IPv4
	    server.sin_port = htons(port);							// Se indica el puerto por donde escucha el servidor indicado por el usario
	 
	 	if (flag3) 													//Si el usuario indico un puerto local especifico
	 	{
    		client.sin_addr.s_addr = INADDR_ANY;				// Recibe clientes con cualquier direccion IP
		    client.sin_family = AF_INET;						// IPv4
		 	client.sin_port = htons(local_port);				// Se indica el puerto por donde se comunica el cliente

		    //Bind
		    if( bind(sock,(struct sockaddr *)&client , sizeof(client)) < 0)
		    {
		        //print the error message
		        perror("bind failed. Error");
		        return 1;
	    	}
	 	}

	    //Connect to remote server
	    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	        perror("connect failed. Error");
	        sleep(5);
	        close(sock);
	        continue;
	    }
	     
	    puts("ATM Connected\n");
	    if(flag)
	    {
	    	flag = 0;
	    	if(send(sock , pp , strlen(pp), MSG_NOSIGNAL) < 0)
	        {
	            puts("Send failed");
	            close(sock);
	        }
	        memset(pp, '\0', 2048);
	        
	        if(send(sock , sp , strlen(sp), MSG_NOSIGNAL) < 0)
	        {
	            puts("Send failed");
	            close(sock);
	        }
	        memset(sp, '\0', 2048);
	    }

	    //keep communicating with server
	    while(1)
	    {
	        if (!input_timeout(STDIN_FILENO,300)){			// Se esperan 5 minutos para el envío de data.

	    		printf("Enter message: ");
		        fgets(message, 2048, stdin);
	    		fprintf (stderr, "\nNo data sent \n");		// Se informa que no se ha enviado data, pero mantiene activa la conexión
	    		fprintf(stderr,"Please enter message: ");	// y se solicita que ingrese un mensaje.

	    	}else{
			printf("Enter message: ");
			fgets(message, 2048, stdin);

			if (message[0] == '\0')						// Cuando se manda como entrada un archivo esto hace que al termniar de leerlo culmine el programa
				return 0;

			//Send some data
			if(send(sock , message , strlen(message), MSG_NOSIGNAL) < 0)
			{
			    puts("Send failed");
			    flag = 1;
			    strcpy(sp,message);
			    close(sock);
			    break;    
			}
			strcpy(pp,message);
			memset(message, '\0', 2048);
		}
	    }
	}
	free(pp);
	free(sp);
	free(ip_srv);
    free(message);
    close(sock);
    return 0;
}
