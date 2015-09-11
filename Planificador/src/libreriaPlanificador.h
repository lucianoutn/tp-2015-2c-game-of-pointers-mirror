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
#define BACKLOG 10
#define PACKAGESIZE 1024

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
	int socket_escucha; 				// Socket de conexiones entrantes
	struct sockaddr_in direccion;		// Datos de la direccion del servidor
	socklen_t tamanio_direccion;		// Tamaño de la direccion
	int CPU[10];						// Sockets de conexiones ACEPTADAS
} conexiones;

//Funcion encargada de acceptar nuevas peticiones de conexion
void escuchar ();

void crearSocket ();

void consola ();

#endif /* SRC_LIBRERIAPLANIFICADOR_H_ */
