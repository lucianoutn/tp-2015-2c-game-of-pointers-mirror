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
#include <SharedLibs/manejoListas.h>
#include <commons/config.h>
#include <unistd.h>

#define BACKLOG 10
#define PACKAGESIZE 1024
#define WORD_SIZE 35


typedef struct{
	char * puertoEscucha;
	char * algoritmoPlanificacion;
} contexto;

contexto miContexto;
int numero_de_pid;

void traigoContexto();



//Estructura que almacenara los datos del PCB de cada proceso
typedef struct PCB{
	int PID;
	//estado del proceso
	unsigned int instructionPointer;
	unsigned int numInstrucciones;
	int prioridad;
	int permisos;
	const char *ruta;
	struct PCB *sig;

}t_pcb;

//Funcion encargada de acceptar nuevas peticiones de conexion
//void escuchar ();

//void crearSocket ();

void encolar (t_pcb *cabecera, t_pcb *valor);

t_pcb desencolar (t_pcb *cabecera);

//inicio consola





void consola ();
//fin consola


#endif /* SRC_LIBRERIAPLANIFICADOR_H_ */
