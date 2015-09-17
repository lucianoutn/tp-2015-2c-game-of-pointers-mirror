/*
 ============================================================================
 Name        : CPU.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Planificador
 ============================================================================
 */

#include "SharedLibs/libreriaCliente.h" //SharedLibs/Debug

#define PACKAGESIZE 1024
#define CANT_CPU 5 //define la cantidad de hilos q se van a crear.lucho

const char *IP = "127.0.0.1";
const char *IP_MEMORIA = "127.0.0.1";  //agrego otra ip xq la mem esta en otra pc.Lucho
const char *PUERTOPLANIFICADOR = "8080";
const char *PUERTOMEMORIA = "8090";

//preparo semaforos.lucho
pthread_mutex_t mutex;
//ptrhead_mutex_lock(&mutex);
//ptrhead_mutex_unlock(&mutex);
//fin semaforos

pthread_t cpu[CANT_CPU];

void iniciaCPU(){



	int socketPlanificador = crearCliente(IP, PUERTOPLANIFICADOR); //conecta con el planificador
	int socketMemoria = crearCliente(IP_MEMORIA, PUERTOMEMORIA);//conecta con la memoria

	pthread_t id= pthread_self(); //retorna el id del hilo q lo llamo
	printf("CPU ID: %d conectado\n", id);

	//Recepcion de instrucciones

	char package[PACKAGESIZE];
	int status;		// Estructura que manjea el status de los recieve.
	printf("CPU ID: %d conectada. Esperando instrucciones:\n", id);

	while(strcmp(package,"salir\n") !=0)
	{
		    status = recv(socketPlanificador, (void*) package, PACKAGESIZE, 0);
			if (status != 0){
				printf("RECIBIDO! =D\n%s", package);
				send(socketMemoria, package, strlen(package) + 1, 0);
			}
			else{
				puts("conexion perdida! =(");
				break;
			}

		}


	close(socketPlanificador);
	close(socketMemoria);	// agrego el cierre del otro socket.lucho

}



//Inicio de funcion principal
int main()
{
	puts("!!!CPU!!"); /* prints !!!CPU!! */

	pthread_mutex_init(&mutex, NULL); //inicializo semaforo.lucho

	//creando los hilos
	int i, err;

	for (i=0; i<CANT_CPU; i++){
		err= pthread_create(&(cpu[i]), NULL, (void*)iniciaCPU, NULL);
		sleep(1);
		if (err != 0)
			printf("no se pudo crear el hilo de cpu :[%s]", strerror(err));
	}

	while(1); //poner sincro
/*	for (i=0; i<CANT_CPU; i++){
		pthread_join(&(cpu[i]), NULL);
	}*/

	return EXIT_SUCCESS;
}
