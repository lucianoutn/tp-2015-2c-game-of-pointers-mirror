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
#include "SharedLibs/manejoListas.h"
#include "SharedLibs/comun.h"
#include <commons/config.h>
#include <commons/string.h>
#include <unistd.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <fcntl.h>           /* para las constantes O_* de los semaforos ipc  */
#include <sys/stat.h>        /* para las constantes de modo de los semaforos ipc */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>


#define BACKLOG 10 //comentar
#define PACKAGESIZE 1024 //comentar
#define WORD_SIZE 35 //comentar

//se crea la lista de todos los pcbs
t_list *lstPcbs;
//Se crea la cola de ready's
t_queue *cola_ready;
//Se crea la cola de bloqueados
t_queue *cola_block;


//Estructura que almacena los datos del archivo de configuracion
typedef struct{
	char * puertoEscucha;
	char * algoritmoPlanificacion;
	int quantum;
	int cantHilosCpus;
} contexto;

contexto miContexto;

typedef struct{
	int socket;
	bool enUso;
} t_cpu;

//struct para conexiones
struct Conexiones {
	int socket_escucha;					// Socket de conexiones entrantes
	struct sockaddr_in direccion;		// Datos de la direccion del servidor
	socklen_t tamanio_direccion;		// Tamaño de la direccion
	//t_cpu CPUS[miContexto.cantHilosCpus];						// Sockets de conexiones ACEPTADAS
	//hago el vector de arriba de forma dinamica
	t_cpu *CPUS; //apunta al primer elemento del vector dinamico
} conexiones;


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

}t_pcb;

//Protocolo de envio Planificador->CPU
typedef struct{
	int tipo_ejecucion;
	//int tamanio_msj;
	key_t clave_pcb;
	key_t clave_ruta;
}t_headcpu;



//Flag
typedef bool flag;

sem_t semSalir;
sem_t semEsperaCPU;
sem_t *semProduccionMsjs;
sem_t ordenIngresada;
sem_t semConsola;
sem_t semCpuLibre;
sem_t semEnvioPcb;
int semVCPU; //vector de semaforos dinamicos compartido


flag CPUenUso; //Flag que permite verificar si una CPU esta en uso o no


int numero_de_pid;

int PID_actual;

void traigoContexto();


//Funcion encargada de acceptar nuevas peticiones de conexion
//void escuchar ();

//void crearSocket ();


void encolar(t_list *, t_queue *);

//Funcion encargada de acceptar nuevas peticiones de conexion
void *escuchar (struct Conexiones* conexion);

void dispatcher();

void enviaACpu(t_cpu *);

t_pcb* procesarPCB(char *);

void preparoHeader(t_headcpu *);

char* estadoActual (int estado); //la uso para el comando PS del planificador.lucho

#endif /* SRC_LIBRERIAPLANIFICADOR_H_ */
