/*
 * libreriaPlanificador.h
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#ifndef SRC_LIBRERIAPLANIFICADOR_H_
#define SRC_LIBRERIAPLANIFICADOR_H_

#include "SharedLibs/libreriaServidor.h" //SharedLibs/Debug
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#define BACKLOG 10
#define PACKAGESIZE 1024

//Estructura que almacenara los datos del PCB de cada proceso
typedef struct PCB{
	int PID;
	//estado del proceso
	unsigned int instructionPointer;
	int prioridad;
	int permisos;
	struct PCB *sig;

}t_pcb;

//Funcion encargada de acceptar nuevas peticiones de conexion
//void escuchar ();

//void crearSocket ();

void encolar (t_pcb *cabecera, t_pcb *valor);

t_pcb desencolar (t_pcb *cabecera);

void consola ();

#endif /* SRC_LIBRERIAPLANIFICADOR_H_ */
