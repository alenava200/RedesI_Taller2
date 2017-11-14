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

typedef struct 										                
{
	char *info;
  	pthread_mutex_t mutex;
}Report;

Report* init_Report();

typedef struct hilo
{
	int fd;
	char *msg;
	char *output;
	Report *R;
}hilo;

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c, port;
    struct sockaddr_in server , client;
    Report *R; 							
	R = init_Report();
	char *output = (char *) malloc(512*sizeof(char));

    if (argc != 5)
    {
    	printf("Wrong Sintaxis.\nPlease use: svr_s -l <puerto_svr_s> -b <archivo_bitácora>\n");
    	exit(-1);
    }
    if ((strcmp(argv[1],"-l") == 0) && (strcmp(argv[3],"-b") == 0))
    {
    	port = atoi(argv[2]);
    	strcpy(output, argv[4]);
    }
    else if((strcmp(argv[1],"-b") == 0) && (strcmp(argv[3],"-l") == 0))
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
    
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( port );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        pthread_t sniffer_thread;
        hilo *h = (hilo*) malloc(sizeof(hilo));
        h->fd = client_sock;
        h->msg = (char*) malloc(2048*sizeof(char));
        h->output = (char*) malloc(512*sizeof(char));
        strcpy(h->output, output);
        h->R = R;

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
 
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor

    hilo *hi = (hilo*) socket_desc;
    int read_size;
	FILE* fp;

    //Receive a message from client
    while( (read_size = read(hi->fd , hi->msg , 2048)) > 0 )
    {
        //Send the message back to client
        int aux = 0;
        aux = alert(hi->msg);
        //write(hi->fd , hi->msg , strlen(hi->msg));
		pthread_mutex_lock(&hi->R->mutex);
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
  		pthread_mutex_unlock(&hi->R->mutex); 
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

Report* init_Report() 
{
	Report *R;
  	R = (Report *) malloc(sizeof(Report));
  	pthread_mutex_init(&R->mutex, NULL);
  	return R;
}