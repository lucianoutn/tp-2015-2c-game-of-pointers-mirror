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
#include "CPU.h"

typedef struct{
	char * ipPlanificador;
	char * puertoPlanificador;
	char * ipMemoria;
	char * puertoMemoria;
	int cantHilos;
	int retardo;
} contexto;

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

sem_t semSalir;

//preparo semaforos.lucho
pthread_mutex_t mutex;
//ptrhead_mutex_lock(&mutex);
//ptrhead_mutex_unlock(&mutex);
//fin semaforos

pthread_t cpu[1];


contexto miContexto;
int numero_de_pid;

void traigoContexto();

void iniciarCPU();

void creoHeader(t_pcb *, t_header*,int,int);

//Funcion que permite verificar el tipo de cada instruccion del mCod
char* interpretarIntruccion(char* instruccion);

//Funcion que le asigna un valor numerico a cada tipo de instruccion
int compararPalabra(char *palabra);

t_pcb* traduceMsj(t_msjRecibido * msj);

#endif /* FUNCCPU_H_ */
