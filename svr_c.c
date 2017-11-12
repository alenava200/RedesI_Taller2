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
    char *server_reply = (char*) malloc(2048*sizeof(char));
    char* ip_srv = (char *) malloc(512*sizeof(char));

    if (argc != 7)
    {
    	printf("Wrong Sintaxis.\nPlease use: svr_s -l <puerto_svr_s> -b <archivo_bitácora>\n");
    	exit(-1);
    }
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
    else
    {
    	printf("Wrong Sintaxis.\nPlease use: svr_s -l <puerto_svr_s> -b <archivo_bitácora>\n");
    	exit(-1);
    }

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);

    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr(ip_srv);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
     
    //keep communicating with server
    while(1)
    {
        printf("Enter message : ");
        fgets(message, 2048, stdin);

        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        if( read(sock , server_reply , 2048) < 0)
        {
            puts("recv failed");
            break;
        }
         
        puts("\nServer reply :");
        puts(server_reply);
        memset(server_reply, '\0', 2048);
        memset(message, '\0', 2048);
    }
    free(message);
    free(server_reply);
    close(sock);
    return 0;
}