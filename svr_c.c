#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdlib.h> //inet_addr
#include <unistd.h> //inet_addr
#include <pthread.h>

 
int main(int argc , char *argv[])
{
    int sock, port;
    struct sockaddr_in server;
    char *message = (char*) malloc(2048*sizeof(char));
    char *ip_srv = (char *) malloc(512*sizeof(char));

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
    
    server.sin_addr.s_addr = inet_addr(ip_srv);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
 
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
        if( send(sock , message , strlen(message) , 0) < 0)
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