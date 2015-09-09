/*
 * libreriaPlanificador.h
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


//struct para conexiones a
struct Conexiones {
	int socket_escucha;
	struct sockaddr_in direccion;
	socklen_t tamanio_direccion;
	int CPU[10];
} conexiones;

//Funcion encargada de acceptar nuevas peticiones de conexion
void *escuchar (struct Conexiones* conexion);

int crearServer (const char *IP, const char *PUERTO);

#endif /* SRC_LIBRERIASERVIDOR_H_ */
