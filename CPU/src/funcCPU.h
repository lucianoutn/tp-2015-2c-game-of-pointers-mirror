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
#include "commons/string.h"
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
#include <sys/poll.h> 		//para el poll del reciv del socket
#include <commons/collections/queue.h>

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
	int quantum; //valor que lee y guarda desde el config
	bool finalizar;
	char *ruta; //ruta del archivo mCod que tiene las instrucciones a procesar
	int tiempo; //tiempo en la cola de bloqueados que tiene que cumplir el proceso
	//Metricas
	time_t t_ejecucion;
	time_t t_espera;
	time_t t_respuesta;
	time_t t_aux1; //auxiliar para medir el tiempo de espera
	time_t t_aux2; //auxiliar para medir el tiempo de respuesta

}t_pcb;

//Protocolo de envio Planificador->CPU
typedef struct{
	int tipo_ejecucion;
	key_t clave_pcb;
	key_t clave_ruta;
}t_headcpu;

//empaqueto los sockets para poder pasarle al hilo mas de un parametro (lucho)
typedef struct {
	//int socketPlanificador;
	int socketMemoria;
	//int numeroCPU;
}t_sockets;

//preparo semaforos.lucho

sem_t semSalir;
sem_t *semProduccionMsjs;
sem_t *semRespuestaCpu;
//int semVCPU; // vector de semaforos dinamico compartidos

//pthread_mutex_t mutex;
//ptrhead_mutex_unlock(&mutex);
//fin semaforos

typedef struct {
	pthread_t hilo;
	int socketPlani;
	int socketMem;
	int numeroCPU;
	int porcentajeUso;
	int cantInstrucEjec;
	int tiempoEjec;
}t_cpu;

t_cpu *CPU;

typedef struct {
	int codigo;
	int pid;
	int pagina;
	char *mensaje;
	int flag;
}t_resultados;

pthread_t hiloTimer;

//pthread_t cpu[0];

t_queue *resultados;

/*		// estructura para el poll:
typedef struct {
		    int fd;         // the socket descriptor
		    short events;   // bitmap of events we're interested in
		    short revents;  // when poll() returns, bitmap of events that occurred
		} t_pollfd;

t_pollfd *pollfd;
*/

int numero_de_pid;

void cargoArchivoConfiguracion();

void creoHeader(t_pcb *, t_header*,int,int);

//t_pcb* traduceMsj(t_msjRecibido * msj);

int palabraAValor(char *palabra);

int procesaInstruccion(char*);

char* procesaMensaje(char*);

int procesaPagina(char*);

void ejecutoPCB(int,int, t_pcb *,int*);

void iniciarCPU(t_cpu *);

int configuroSocketsYLogs ();

void timer ();

void comandoCpu (int);

void imprimeResultados(t_resultados *);

t_resultados* resultado(int codigo, int pid, int pagina, char* mensaje, int flag);

#endif /* FUNCCPU_H_ */
