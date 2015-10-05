/*
 ============================================================================
 Name        : CPU.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Planificador
 ============================================================================
 */

#include "CPU.h"
#include "funcCPU.h"

#define PACKAGESIZE 1024



//Inicio de funcion principal
int main()
{
	puts("!!!CPU!!"); /* prints !!!CPU!! */

	traigoContexto();
	//creoLogger();
	log_info(logger, "Inicio Log CPU", NULL);

	pthread_mutex_init(&mutex, NULL); //inicializo semaforo.lucho
	semSalir.__align =0;
	//creando los hilos
	int i, err;

//	for (i=0; i<CANT_CPU; i++){
		//err= pthread_create(&(cpu[0]), NULL, (void*)iniciaCPU, NULL);
		//sleep(1);
		//if (err != 0)
		//	printf("no se pudo crear el hilo de cpu :[%s]", strerror(err));
//	}
		iniciarCPU();
	//	while(1);
	sem_wait(&semSalir); //poner sincro
/*	for (i=0; i<CANT_CPU; i++){
		pthread_join(&(cpu[i]), NULL);
	}*/

	/*
	 * La cpu recibe el PCB del planificador (luego de que este halla recibido la instruccion correr PATH
	 * 	PCB=recv(puntero a la estructura del PCB que creo el plani)
	 * 	PCB.PID=PID_actual
		PCB.instructionPointer=0
		char **instrucciones= (char**)malloc(sizeof(leermCod(PATH, PCB.numInstrucciones));
		instrucciones = (leermCod(PATH, PCB.numInstrucciones);

		hace el switch y segun el caso envia el playload y dps la instruccion (afuera del switch).
		while(!strcmp(instruccion[num],"fin")) //envia hasta que llegue a la ultima instrucciones
		{
			switch(tipo de instruccion)
			{
				case n:
					send(payload) que corresponda segun el caso a la memoria
			}
			send(instrucciones[instructionPointer]);
			instructionPointer++;
			y espera a lo que tenga que hacer la instruccion o a algun valor que devuelva la memoria
		}
	 */

	return EXIT_SUCCESS;
}





