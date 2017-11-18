
#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

#include <pthread.h>

// Estructura que sirve para pasar los parametros que los hilos se deben llevar a la funcion donde los hilos reciben la informacion de los clientes
typedef struct hilo 									
{														
	int fd; 							// File descriptor
	char *msg;							// Cadena de caracteres donde se almacenan los mensajes que recibe el servidor
	char *output; 						// Cadena de caracteres donde se almacena el Nombre del archivo de salida
	pthread_mutex_t mutex; 				// Mutex que evita que varios hilos escriban sobre el archivo de salida al mismo tiempo
}hilo;

//the thread function
void *connection_handler(void *);
int alert(char *msg);

#endif
