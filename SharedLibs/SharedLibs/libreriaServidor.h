/*
 * libreriaServidor.h
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#ifndef SRC_LIBRERIASERVIDOR_H_
#define SRC_LIBRERIASERVIDOR_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>


//struct para conexiones
struct Conexiones {
	int socket_escucha;					// Socket de conexiones entrantes
	struct sockaddr_in direccion;		// Datos de la direccion del servidor
	socklen_t tamanio_direccion;		// Tamaño de la direccion
	int CPU[10];						// Sockets de conexiones ACEPTADAS
} conexiones;

//Funcion encargada de acceptar nuevas peticiones de conexion
void *escuchar (struct Conexiones* conexion);

int crearServer (const char *IP, const char *PUERTO);

#endif /* SRC_LIBRERIASERVIDOR_H_ */
