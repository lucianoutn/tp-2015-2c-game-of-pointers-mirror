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
#include <fcntl.h>           /* para las constantes O_* de los semaforos ipc  */
#include <sys/stat.h>        /* para las constantes de modo de los semaforos ipc */
#include <sys/ipc.h>
#include <sys/shm.h>


#define BACKLOG 10 //comentar
#define PACKAGESIZE 1024 //comentar
#define WORD_SIZE 35 //comentar

t_list *lstPcbs;


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
	//int tamanio_msj;
	key_t clave_pcb;
	key_t clave_ruta;
}t_headcpu;



//Estructura que almacena los datos del archivo de configuracion
typedef struct{
	char * puertoEscucha;
	char * algoritmoPlanificacion;
	int quantum;
} contexto;

//Flag
typedef bool flag;

sem_t semSalir;
sem_t semEsperaCPU;
sem_t *semProduccionMsjs;
sem_t *semRespuestaCpu;
sem_t ordenIngresada;
sem_t semConsola;
sem_t semCpuLibre;
sem_t semEnvioPcb;

contexto miContexto;

flag CPUenUso; //Flag que permite verificar si una CPU esta en uso o no

int numero_de_pid;

int PID_actual;

void traigoContexto();


//Funcion encargada de acceptar nuevas peticiones de conexion
//void escuchar ();

//void crearSocket ();


void encolar(t_list *, t_queue *);

void dispatcher(t_queue *);

void enviaACpu(t_cpu);

t_pcb* procesarPCB(char *);

void preparoHeader(t_headcpu *);

#endif /* SRC_LIBRERIAPLANIFICADOR_H_ */
