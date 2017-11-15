#include <stdio.h> 
#include <string.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h>

 
int main(int argc , char *argv[])
{
    int sock, port;
    struct sockaddr_in server;
    char *message = (char*) malloc(2048*sizeof(char)); 		// Cadena de caracteres que almacena los mensajes que se transmitiran al servidor
    char *ip_srv = (char *) malloc(512*sizeof(char)); 		// Cadena de caracteres donde se almacena la direccion ip del servidor indicado por el usuario

    // Selector que permite que el programa funcione con la sintaxis deseada vr_c -d <nombre_módulo_central> -p <puerto_svr_s> [-l <puerto_local>]
    switch(argc) 									
	{
		case 5:	
				if ((strcmp(argv[1],"-d") == 0) && (strcmp(argv[3],"-p") == 0))
			    {
			    	strcpy(ip_srv, argv[2]); 
			    	port = atoi(argv[4]);
			    }
			    else if((strcmp(argv[1],"-p") == 0) && (strcmp(argv[3],"-d") == 0))
			    {
			    	strcpy(ip_srv, argv[4]); 
			    	port = atoi(argv[2]);
			    }
			    break;

		case 7:	
			    if ((strcmp(argv[1],"-d") == 0) && (strcmp(argv[3],"-p") == 0) && (strcmp(argv[5],"-l") == 0))
			    {
			    	strcpy(ip_srv, argv[2]); 
			    	port = atoi(argv[4]);
			    }
			    else if((strcmp(argv[1],"-d") == 0) && (strcmp(argv[3],"-l") == 0) && (strcmp(argv[5],"-p") == 0))
			    {
			    	strcpy(ip_srv, argv[2]); 
			    	port = atoi(argv[6]);
			    }
			    else if((strcmp(argv[1],"-p") == 0) && (strcmp(argv[3],"-d") == 0) && (strcmp(argv[5],"-l") == 0))
			    {
			    	strcpy(ip_srv, argv[4]); 
			    	port = atoi(argv[2]);
			    }
			    else if((strcmp(argv[1],"-p") == 0) && (strcmp(argv[3],"-l") == 0) && (strcmp(argv[5],"-d") == 0))
			    {
			    	strcpy(ip_srv, argv[6]); 
			    	port = atoi(argv[2]);
			    }
			    else if((strcmp(argv[1],"-l") == 0) && (strcmp(argv[3],"-d") == 0) && (strcmp(argv[5],"-p") == 0))
			    {
			    	strcpy(ip_srv, argv[4]); 
			    	port = atoi(argv[6]);
			    }
			    else if((strcmp(argv[1],"-l") == 0) && (strcmp(argv[3],"-p") == 0) && (strcmp(argv[5],"-d") == 0))
			    {
			    	strcpy(ip_srv, argv[6]); 
			    	port = atoi(argv[4]);
			    }
				break;

		default:
				printf("Wrong Sintaxis.\nsvr_c -d <nombre_módulo_central> -p <puerto_svr_s> [-l <puerto_local>]\n");
    			exit(-1);
	}

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);

    if (sock == -1)
        printf("Could not create socket");
    
    server.sin_addr.s_addr = inet_addr(ip_srv); 			// Se indica la direccion del servidor indicado por usuario
    server.sin_family = AF_INET;							// IPv4
    server.sin_port = htons(port);							// Se indica el puerto por donde escucha el servidor indicado por el usario
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        close(sock);
        return 1;
    }
     
    puts("ATM Connected\n");
     
    //keep communicating with server
    while(1)
    {
        
        printf("Enter message: ");
        fgets(message, 2048, stdin);

        //Send some data
        if( write(sock , message , strlen(message)) < 0)
        {
            puts("Send failed");
            close(sock);
            return 1;
        }
        memset(message, '\0', 2048);
    }
    free(message);
    close(sock);
    return 0;
}