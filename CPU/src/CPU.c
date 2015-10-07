/*
 ============================================================================
 Name        : CPU.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - CPU
 ============================================================================
 */


#include "funcCPU.h"

#define PACKAGESIZE 1024

//Inicio de funcion principal
int main()
{
	int socketPlanificador, socketMemoria/*, i, err*/;
	semSalir.__align =0;

	if (configuroSocketsYLogs(&socketPlanificador, &socketMemoria) == 1) //Preparo las configuraciones bascias para ejecutar la CPU
		puts("¡¡¡CPU CONECTADA!!!");
	else
		return EXIT_FAILURE;

	pthread_mutex_init(&mutex, NULL); //inicializo semaforo.lucho
	//preparo semaforos.lucho
	//pthread_mutex_t mutex;
	//ptrhead_mutex_lock(&mutex);
	//ptrhead_mutex_unlock(&mutex);
	//fin semaforos


	//creando los hilos


//	for (i=0; i<CANT_CPU; i++){
		//err= pthread_create(&(cpu[0]), NULL, (void*)iniciaCPU, NULL);
		//sleep(1);
		//if (err != 0)
		//	printf("no se pudo crear el hilo de cpu :[%s]", strerror(err));
//	}
	iniciarCPU(socketPlanificador,socketMemoria);

	sem_wait(&semSalir);

	return EXIT_SUCCESS;
}


/*Configuraciones basicas de los Sockets
 * y los Logs para el CPU
 */
int configuroSocketsYLogs (int *socketPlanificador,int *socketMemoria){
	cargoArchivoConfiguracion(); //carga las configuraciones basicas
	creoLogger(0);  //recive 0 para log solo x archivo| recive 1 para log x archivo y x pantalla
	log_info(logger, "Inicio Log CPU", NULL);
	puts("Conexion con el Planificador");
	*socketPlanificador = crearCliente(configuracion.ipPlanificador, configuracion.puertoPlanificador); //conecta con el planificador
	puts("Conexion con la Memoria");
	*socketMemoria = crearCliente(configuracion.ipMemoria, configuracion.puertoMemoria);//conecta con la memoria
	if (*socketPlanificador < 0)
		return 0;
	else {
		if (*socketMemoria < 0)
			return 0;
		else
			return 1;
	}
}



