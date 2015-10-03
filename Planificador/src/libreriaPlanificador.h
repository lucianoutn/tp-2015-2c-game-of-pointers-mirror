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
#include <SharedLibs/comun.h>
#include <commons/config.h>
#include <commons/string.h>
#include <unistd.h>
#include <semaphore.h>
#include <commons/collections/queue.h>

#define BACKLOG 10
#define PACKAGESIZE 1024
#define WORD_SIZE 35

typedef struct PCB {
	int PID;
	//estado del proceso
	unsigned int instructionPointer;
	unsigned int numInstrucciones;
	int prioridad;
	int permisos;
	char * ruta;
}t_pcb;


//Protocolo de envio Planificador -> CPU

typedef struct{
	int tipo_ejecucion;
	int tamanio_msj;
}t_headcpu;

typedef struct MSJ {
	t_headcpu headMSJ;
	int PID;
	//estado del proceso
	unsigned int instructionPointer;
	unsigned int numInstrucciones;
	int prioridad;
	int permisos;
	char ruta[20];
	//t_pcb  pcbMSJ;
}t_msj;

typedef struct{
	char * puertoEscucha;
	char * algoritmoPlanificacion;
} contexto;

typedef bool flag;

sem_t semSalir;

contexto miContexto;

flag CPUenUso;

int numero_de_pid;

int max_PID;

void traigoContexto();



//Estructura que almacenara los datos del PCB de cada proceso

//Funcion encargada de acceptar nuevas peticiones de conexion
//void escuchar ();

//void crearSocket ();

void encolar (t_pcb *cabecera, t_pcb *valor);

t_pcb desencolar (t_pcb *cabecera);

//inicio consola

void procesarPCB (t_queue*, char *, int);

int consola (t_queue *);

void iniciarPlanificador();
//fin consola

#endif /* SRC_LIBRERIAPLANIFICADOR_H_ */
