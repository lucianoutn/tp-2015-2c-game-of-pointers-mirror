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

#define PACKAGESIZE 1024

//preparo semaforos.lucho
pthread_mutex_t mutex;
//ptrhead_mutex_lock(&mutex);
//ptrhead_mutex_unlock(&mutex);
//fin semaforos

pthread_t cpu[1];


//Inicio de funcion principal
int main()
{
	puts("!!!CPU!!"); /* prints !!!CPU!! */

	traigoContexto();

	pthread_mutex_init(&mutex, NULL); //inicializo semaforo.lucho

	//creando los hilos
	int i, err;

//	for (i=0; i<CANT_CPU; i++){
		err= pthread_create(&(cpu[0]), NULL, (void*)iniciaCPU, NULL);
		sleep(1);
		if (err != 0)
			printf("no se pudo crear el hilo de cpu :[%s]", strerror(err));
//	}

	while(1); //poner sincro
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

void iniciaCPU(){


	int socketPlanificador = crearCliente(miContexto.ipPlanificador, miContexto.puertoPlanificador); //conecta con el planificador
	int socketMemoria = crearCliente(miContexto.ipMemoria, miContexto.puertoMemoria);//conecta con la memoria

	pthread_t id= pthread_self(); //retorna el id del hilo q lo llamo
	printf("CPU ID: %d conectado\n", (pthread_t)id);

	//Recepcion de instrucciones

	t_pcb * pcb=malloc(sizeof(t_pcb));
	int status=1;		// Estructura que manjea el status de los recieve.
	printf("CPU ID: %d conectada. Esperando instrucciones:\n", (pthread_t)id);

	t_header *header = malloc(sizeof(t_header));
	t_headcpu *headcpu = malloc(sizeof(t_headcpu));

	while(status!=0)
	{
		status = recv(socketPlanificador, headcpu, sizeof(t_headcpu),0);
		if(status!=0)
		{
			switch (headcpu->tipo_ejecucion)
			{
			case 0:
				//FINALIZO CONEXIONES
				printf("Recibi salir, cierro conexiones");
				break;
			case 1:
				status = recv(socketPlanificador, pcb, headcpu->tamanio_msj, 0);
				if (status != 0)
				{
					printf("Recibi PCB");
					creoHeader(pcb,header);
					send(socketMemoria, header, sizeof(t_header), 0);
				}
			}

		}
		else
		{
			puts("Conexion perdida!");
			break;
		}

	}
	free(header);
	free(headcpu);
	close(socketPlanificador);
	close(socketMemoria);	// agrego el cierre del otro socket.lucho

}
