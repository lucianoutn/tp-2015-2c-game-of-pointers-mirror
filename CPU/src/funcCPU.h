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

typedef struct{
	char * ipPlanificador;
	char * puertoPlanificador;
	char * ipMemoria;
	char * puertoMemoria;
	int cantHilos;
	int retardo;
} contexto;

typedef struct PCB {
	int PID;
	//estado del proceso
	unsigned int instructionPointer;
	unsigned int numInstrucciones;
	int prioridad;
	int permisos;
	char* ruta;
	struct PCB *sig;
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


//Protocolo de envio Planificador -> CPU

typedef struct{
	int tipo_ejecucion;
	int tamanio_msj;
}t_headcpu;

//Estructura que almacenara los datos del PCB de cada proceso


contexto miContexto;
int numero_de_pid;

void traigoContexto();

void iniciaCPU();

void creoHeader(t_pcb *, t_header*,int,int);

//Funcion que permite verificar el tipo de cada instruccion del mCod
char* interpretarIntruccion(char* instruccion);

//Funcion que le asigna un valor numerico a cada tipo de instruccion
int compararPalabra(char *palabra);

#endif /* FUNCCPU_H_ */
