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
	semSalir.__align =0;
	semProduccionMsjs = sem_open("semPlani", 0);


	//semRespuestaCpu = sem_open("semCPU", O_CREAT, 0666, 0);//inicializo sem prod-consum, el 0_creat es para evitar q se inicialize en el otro proceso
	if (configuroSocketsYLogs() == 1) //Preparo las configuraciones bascias para ejecutar la CPU
		puts("¡¡¡CPU!!!");
	else
		return EXIT_FAILURE;

	pthread_mutex_init(&mutex, NULL); //inicializo semaforo.lucho


	//creando los hilos
	//puts("CREO HILOS DE CPU");

	int i, err;
	for (i=1; i<=configuracion.cantHilos; i++){
		CPU[i].numeroCPU = i;
		err= pthread_create(&CPU[i].hilo, NULL, (void*)iniciarCPU, &CPU[i]);
		//sleep(1);
		if (err != 0)
		printf("no se pudo crear el hilo de cpu :[%s]", strerror(err));
	}

	puts("Cree las CPUS correctamente");

	sem_wait(&semSalir);
	//pthread_join(CPU[0].hilo,NULL);
	puts("CHAU");
	close(sockets->socketMemoria);
	for (i=1; i<=configuracion.cantHilos; i++){
			//close(CPU[i].socketPlani);
			pthread_cancel(CPU[i].hilo);
	free(CPU);
	//free(sockets);

	return EXIT_SUCCESS;
}





