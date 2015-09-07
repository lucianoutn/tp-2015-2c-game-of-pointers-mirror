/*
 * libreriaPlanificador.h
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#ifndef SRC_LIBRERIAPLANIFICADOR_H_
#define SRC_LIBRERIAPLANIFICADOR_H_

#define IP "127.0.0.1"
#define PUERTO "8080"

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
	int socket_escucha;
	struct sockaddr_in direccion;
	socklen_t tamanio_direccion;
	int CPU[10];
} conexiones;

//Funcion encargada de acceptar nuevas peticiones de conexion
void* escuchar (struct Conexiones* conexion);

int crearSocket ();

#endif /* SRC_LIBRERIAPLANIFICADOR_H_ */
