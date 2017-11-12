#include <stdio.h>
#include <string.h>    
#include <stdlib.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>    
#include <pthread.h> 

typedef struct mensaje {
        int serial;
        char fecha[10];
        char hora[6];
        int ID;
        int codigo;
        char patron[1000];
        char info[1000];
    } MSG;
 
//Función de los Hilos
void *connection_handler(void *);
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
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
    puts("Esperando Conexion...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    puts("Esperando Conexion...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Conexion Aceptada");
         
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("No se pudo crear el hilo");
            return 1;
        }
         
        
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}
 
/*
 * Conexión con el cliente
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    MSG message , client_message;
     
    
     
    //message = "Type \n";
    //write(sock , message , strlen(message));
     
    //Recibe mensaje del cliente
    while( (read_size = recv(sock , &client_message , sizeof(client_message) , 0)) > 0 )
    {
        //end of string marker
		//client_message[read_size] = '\0';
		
		//Envía el mensaje al cliente
        write(sock , &client_message , strlen(&client_message));
		
		//Limpiar Buffer del mensaje
		//memset(client_message, 0, 2000);
    }
     
    if(read_size == 0)
    {
        puts("Cliente Desconectado");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    return 0;
} 
