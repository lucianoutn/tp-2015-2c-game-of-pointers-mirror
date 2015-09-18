/*
 ============================================================================
 Name        : Planificador.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Planificador
 ============================================================================
 */

#include "libreriaPlanificador.h"
#include <commons/collections/list.h>

//Estructura que almacenara los datos del PCB de cada proceso
typedef struct PCB{
	int PID;
	//estado del proceso
	unsigned int instructionPointer;
	int prioridad;
	int permisos;
	struct PCB *sig;

}t_pcb;

//A continuancion las funciones basicas para crear una nueva cola FIFO
//Funcion que permite añadir elementos a la cola
void encolar (t_pcb *cabecera, t_pcb *valor)
{
	t_pcb *nuevo = malloc(sizeof(t_pcb));
	nuevo=valor;

	if (cabecera==NULL)
	{
		nuevo->sig=nuevo;
	}
	else
	{
		nuevo->sig= cabecera->sig;
		cabecera->sig= nuevo;
	}

	cabecera=nuevo;
};

//Funcion que permite quitar elementos de la cola, devuelve el nodo que saca.
t_pcb desencolar (t_pcb *cabecera)
{
	t_pcb *ret = malloc(sizeof(t_pcb));
	ret= cabecera;
	if(cabecera==cabecera->sig)
	{
		free (cabecera);
	}

	else
	{
		t_pcb *aux = malloc(sizeof(t_pcb));
		aux= cabecera->sig;
		cabecera->sig=aux->sig;
		free(aux);
	}

	return *ret;
};

int main() {
	puts("!!!Planificador!!!"); /* prints !!!Planificador!!! */

	/*
	 * Funcion que crea un socket nuevo y lo prepara para escuchar conexiones entrantes a travez del puerto PUERTO
	 * y lo almacena en la variable conexiones del tipo struct Conexiones
	 */
	crearSocket();

	/*
	 * Se crea un hilo nuevo que se queda a la espera de nuevas conexiones del CPU
	 * y almacena los sockets de las nuevas conexiones en la variable conexiones.CPU[]
	 */
	pthread_t hilo_conexiones, hilo_consola;
	if(pthread_create(&hilo_conexiones, NULL, (void*)escuchar,NULL)<0)
		perror("Error HILO ESCUCHAS!");

	/*
	 * REEMPLAZAR MAS ADELANTE (SINCRONIZACION)
	 */
	puts("ESPERANDO CONEXIONES....\n");
	while(conexiones.CPU[0] <= 0){
	}

	//Crea un hilo para la consola y espera a que termine para finalizar el programa
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);
	pthread_join(hilo_consola, NULL);

	//cierra los sockets
	close(conexiones.socket_escucha);
	int i=0;
	while(i<10)
	{
		close(conexiones.CPU[i++]);
	}

	return EXIT_SUCCESS;
}


