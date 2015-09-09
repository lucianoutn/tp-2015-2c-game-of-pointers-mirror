/*
 ============================================================================
 Name        : Swap.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Swap
 ============================================================================
 */

#include "libreriaServidor.h"

#define BACKLOG 10
#define PACKAGESIZE 1024

const char *IP= "127.0.0.1";
const char *PUERTOSWAP= "9000";

int main(void) {
	puts("!!!Swap!!!"); /* prints !!!Swap!!! */

	int listenningSocket= crearServer (IP, PUERTOSWAP);

	//Estructura que tendra los datos de la conexion del cliente MEMORIA
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int L = listen(listenningSocket, BACKLOG);
	if (L==-1)
		perror("LISTEN");


	int memSocket = accept(listenningSocket, (struct sockaddr *) &addr,	&addrlen);
		printf("Conexion aceptada Socket= %d \n",memSocket);

	//Recepcion de instrucciones
	char package[PACKAGESIZE];
	int status;		// Estructura que manjea el status de los recieve.
	printf("ADM de Memoria conectado. Esperando instrucciones:\n");

	while(strcmp(package,"salir\n") !=0)
	{
		status = recv(memSocket, (void*) package, PACKAGESIZE, 0);
		if (status != 0){
			printf("RECIBIDO! =D\n%s", package);
		}
		else{
			puts("conexion perdida! =(");
			break;
		};
	}


	close(memSocket);
	close(listenningSocket);


	return EXIT_SUCCESS;
}
