/*
 * libreriaPlanificador.h
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#ifndef SRC_LIBRERIAPLANIFICADOR_H_
#define SRC_LIBRERIAPLANIFICADOR_H_

//Libreria que usa el planificador para funcionar
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
#include <fcntl.h>           /* para las constantes O_*  */
#include <sys/stat.h>        /* para las constantes de modo */



#define BACKLOG 10 //comentar
#define PACKAGESIZE 1024 //comentar
#define WORD_SIZE 35 //comentar
#define MAX_CPUS 1

//tipo de dato t_pcb que es una estructura que almacena el PCB de cada proceso
typedef struct PCB {
	int PID; //numero del proceso
	//estado del proceso
	unsigned int instructionPointer; //numero de instruccion actual
	unsigned int numInstrucciones; //numero total de instrucciones
	int prioridad; // prioridad (usar mas adelante)
	int permisos; // sin uso por ahora
	char * ruta; //ruta del archivo mCod que tiene las instrucciones a procesar
	//struct PCB *sig;
}t_pcb;


//Protocolo de envio Planificador->CPU
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

//Estructura que almacena los datos del archivo de configuracion
typedef struct{
	char * puertoEscucha;
	char * algoritmoPlanificacion;
} contexto;

//Flag
typedef bool flag;

sem_t semSalir;
sem_t semEsperaCPU;
sem_t *semProduccionMsjs;

contexto miContexto;

flag CPUenUso; //Flag que permite verificar si una CPU esta en uso o no

int numero_de_pid;

int max_PID;

void traigoContexto();


//Funcion encargada de acceptar nuevas peticiones de conexion
//void escuchar ();

//void crearSocket ();

//Funcion que permite crear una cola
void encolar (t_pcb *cabecera, t_pcb *valor);

//Funcion que permite quitar un elemento de la cola
t_pcb desencolar (t_pcb *cabecera);

//inicio consola

void procesarPCB (t_queue*, char *, int);


void iniciarPlanificador();
//fin consola

#endif /* SRC_LIBRERIAPLANIFICADOR_H_ */
