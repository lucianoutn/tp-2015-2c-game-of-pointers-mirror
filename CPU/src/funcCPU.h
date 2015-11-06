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
#include "SharedLibs/manejoListas.h"
#include "SharedLibs/comun.h"
#include "commons/process.h"
#include <commons/config.h>
#include <semaphore.h>
#include "mAnsisOp.h"
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>           // para las constantes O_* de los semaforos ipc
#include <sys/stat.h>      //  para las constantes de modo de los semaforos ipc
#include <linux/unistd.h>   //para alguna syscall


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

config configuracion;

//tipo de dato t_pcb que es una estructura que almacena el PCB de cada proceso
typedef struct PCB {
	int PID; //numero del proceso
	/*
	 * 0 new
	 * 1 ready
	 * 2 executing
	 * 3 block
	 * 4 finish
	 */
	unsigned int estado;
	unsigned int instructionPointer; //numero de instruccion actual
	unsigned int numInstrucciones; //numero total de instrucciones
	int prioridad; // prioridad (usar mas adelante)
	int permisos; // sin uso por ahora
	char *ruta; //ruta del archivo mCod que tiene las instrucciones a procesar
	bool finalizar;

}t_pcb;

//Protocolo de envio Planificador->CPU
typedef struct{
	int tipo_ejecucion;
	key_t clave_pcb;
	key_t clave_ruta;
}t_headcpu;

//empaqueto los sockets para poder pasarle al hilo mas de un parametro (lucho)
typedef struct {
	int socketPlanificador;
	int socketMemoria;
	int numeroCPU;
}t_sockets;

//preparo semaforos.lucho

sem_t semSalir;
sem_t *semProduccionMsjs;
sem_t *semRespuestaCpu;
int semVCPU; // vector de semaforos dinamico compartidos

pthread_mutex_t mutex;
//ptrhead_mutex_unlock(&mutex);
//fin semaforos



pthread_t cpu[0];

t_sockets *sockets;

int numero_de_pid;

void cargoArchivoConfiguracion();

void creoHeader(t_pcb *, t_header*,int,int);

//t_pcb* traduceMsj(t_msjRecibido * msj);

int palabraAValor(char *palabra);

int procesaInstruccion(char*,int *);

void ejecutoPCB(int,int, t_pcb *);

void iniciarCPU(t_sockets *);

int configuroSocketsYLogs (t_sockets *);

#endif /* FUNCCPU_H_ */
