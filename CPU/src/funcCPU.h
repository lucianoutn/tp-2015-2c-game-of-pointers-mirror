/*
 * funcCPU.h
 *
 *  Created on: 30/9/2015
 *      Author: utnso
 */

#ifndef FUNCCPU_H_
#define FUNCCPU_H_

#include "SharedLibs/libreriaCliente.h" //SharedLibs/Debug
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <SharedLibs/manejoListas.h>
#include <SharedLibs/comun.h>
#include <commons/config.h>
#include <semaphore.h>
#include "mAnsisOp.h"

typedef int pid;

pid PID_actual;

typedef bool flag;

typedef struct{
	char * ipPlanificador;
	char * puertoPlanificador;
	char * ipMemoria;
	char * puertoMemoria;
	int cantHilos;
	int retardo;
} config;

//tipo de dato t_pcb que es una estructura que almacena el PCB de cada proceso
typedef struct PCB {
	int PID; //numero del proceso
	//estado del proceso
	unsigned int instructionPointer; //numero de instruccion actual
	unsigned int numInstrucciones; //numero total de instrucciones
	int prioridad; // prioridad (usar mas adelante)
	int permisos; // sin uso por ahora
	char * ruta; //ruta del archivo mCod que tiene las instrucciones a procesar
}t_pcb;

typedef struct MSJ {
	int PID;
	//estado del proceso
	unsigned int instructionPointer;
	unsigned int numInstrucciones;
	int prioridad;
	int permisos;
	char ruta[20];
	struct PCB *sig;
}t_msjRecibido;


//Protocolo de envio Planificador->CPU
typedef struct{
	int tipo_ejecucion;
	int tamanio_msj;
}t_headcpu;

//empaqueto los sockets para poder pasarle al hilo mas de un parametro (lucho)
typedef struct {
	int socketPlanificador;
	int socketMemoria;
}t_sockets;


sem_t semSalir;

//preparo semaforos.lucho
pthread_mutex_t mutex;
//ptrhead_mutex_lock(&mutex);
//ptrhead_mutex_unlock(&mutex);
//fin semaforos

config configuracion;

pthread_t cpu[0];

t_sockets sockets;

int numero_de_pid;

void cargoArchivoConfiguracion();

void creoHeader(t_pcb *, t_header*,int,int);

t_pcb* traduceMsj(t_msjRecibido * msj);

int palabraAValor(char *palabra);

int procesaInstruccion(char*,int *);

void procesoMSJ(int,t_pcb *);

void iniciarCPU(t_sockets *);

int configuroSocketsYLogs (int *,int *);

#endif /* FUNCCPU_H_ */
