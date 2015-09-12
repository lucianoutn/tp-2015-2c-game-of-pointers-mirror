/*
 ============================================================================
 Name        : CPU.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Planificador
 ============================================================================
 */

#include "libreriaCliente.h"

#define PACKAGESIZE 1024

const char *IP = "127.0.0.1";
const char *IP_MEMORIA = "127.0.0.1";  //agrego otra ip xq la mem esta en otra pc.Lucho
const char *PUERTOPLANIFICADOR = "8080";
const char *PUERTOMEMORIA = "8090";


//Inicio de funcion principal
int main()
{
	puts("!!!CPU!!"); /* prints !!!CPU!! */


	int socketPlanificador = crearClientePlani(IP, PUERTOPLANIFICADOR); //conecta con el planificador
	int socketMemoria = crearClienteMem(IP_MEMORIA, PUERTOMEMORIA);//conecta con la memoria

//Recepcion de instrucciones

	char package[PACKAGESIZE];
	int status;		// Estructura que manjea el status de los recieve.
	printf("CPU conectada. Esperando instrucciones:\n");

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


	return EXIT_SUCCESS;
}
