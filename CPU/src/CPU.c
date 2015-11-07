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
	sockets = malloc(sizeof(t_sockets));
	//int socketPlanificador, socketMemoria;
	semSalir.__align =0;
	semProduccionMsjs = sem_open("semPlani", 0);


	//semRespuestaCpu = sem_open("semCPU", O_CREAT, 0644, 0);//inicializo sem prod-consum, el 0_creat es para evitar q se inicialize en el otro proceso
	if (configuroSocketsYLogs(sockets) == 1) //Preparo las configuraciones bascias para ejecutar la CPU
		puts("¡¡¡CPU!!!");
	else
		return EXIT_FAILURE;

	pthread_mutex_init(&mutex, NULL); //inicializo semaforo.lucho

/*	//vector dinamico de semaforos. uno x cada cpu
	key_t keySem;
	keySem = ftok ("/bin/ls", 33);	//genero una clave para identificar el array de semaforos en los otros procesos
	semVCPU = semget (keySem, configuracion.cantHilos, 0); // abro (ya creados x plani) tanto semaforos como hilos tenga
*/

	/*
	 semVCPU = (sem_t*)malloc(sizeof(sem_t) * (configuracion.cantHilos));
	 int h;
	 for (h=0; h < configuracion.cantHilos; h++){
	 inicSemVCPU(&(semVCPU[h]));
	 //semVCPU[h]= sem_open("semVCPU", O_CREAT, 0644, 0);
	 }
*/
	//creando los hilos


	int i, err;
	for (i=0; i<=0/*i<configuracion.cantHilos*/; i++){
		sockets->numeroCPU = i;
		err= pthread_create(&(cpu[i]), NULL, (void*)iniciarCPU,sockets);
		//sleep(1);
		if (err != 0)
		printf("no se pudo crear el hilo de cpu :[%s]", strerror(err));
	}

	pthread_join(cpu[0], NULL);

	//iniciarCPU(socketPlanificador,socketMemoria); //sin hilos

	sem_wait(&semSalir);

	return EXIT_SUCCESS;
}





