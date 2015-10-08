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
	//int socketPlanificador, socketMemoria;
	semSalir.__align =0;
	semProduccionMsjs = sem_open("semPlani", 0);
	if (configuroSocketsYLogs(&(sockets.socketPlanificador), &(sockets.socketMemoria)) == 1) //Preparo las configuraciones bascias para ejecutar la CPU
		puts("¡¡¡CPU!!!");
	else
		return EXIT_FAILURE;

	pthread_mutex_init(&mutex, NULL); //inicializo semaforo.lucho


	//creando los hilos


	int i, err;
	for (i=0; i<configuracion.cantHilos; i++){
		err= pthread_create(&(cpu[i]), NULL, (void*)iniciarCPU,&sockets);
		//sleep(1);
		if (err != 0)
		printf("no se pudo crear el hilo de cpu :[%s]", strerror(err));
	}

	pthread_join(cpu[0], NULL);

	//iniciarCPU(socketPlanificador,socketMemoria); //sin hilos

	sem_wait(&semSalir);

	return EXIT_SUCCESS;
}





