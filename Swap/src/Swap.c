/*
 ============================================================================
 Name        : Swap.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Swap
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PUERTO "9000"
#define BACKLOG 10
#define PACKAGESIZE 1024

int main(void) {
	puts("!!!Swap!!!"); /* prints !!!Swap!!! */

	//Configuracion del socket
	struct addrinfo hints; //estructura que almacena los datos de conexion del Swap
	struct addrinfo *serverInfo; //estructura que almacena los datos de conexion de la Memoria

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion



	//se crea un nuevo socket que se utilizara para la escucha de la Memoria
	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	//se comprueba que el socket se creo correctamente
	if (listenningSocket == -1)
		perror("SOCKET");

	//se comprueba que la asociacion fue exitosa
	int B = bind(listenningSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	if (B==-1)
		perror("BIND");

	freeaddrinfo(serverInfo);
	//lo pongo a escuchar

	//Estructura que tendra los datos de la conexion del cliente MEMORIA
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int L = listen(listenningSocket, BACKLOG);
	if (L==-1)
		perror("LISTEN");

	int socketSwap;
	socketSwap = accept(listenningSocket, (struct sockaddr *) &addr,	&addrlen);
		printf("Conexion aceptada Socket= %d \n",socketSwap);

	//Recepcion de instrucciones
	char package[PACKAGESIZE];
	int status;		// Estructura que manjea el status de los recieve.
	printf("ADM de Memoria conectado. Esperando instrucciones:\n");

	while(strcmp(package,"salir\n") !=0)
	{
		status = recv(socketSwap, (void*) package, PACKAGESIZE, 0);
		if (status != 0){
			printf("RECIBIDO! =D\n%s", package);
		}
		else{
			puts("conexion perdida! =(");
			break;
		};
	}


	close(socketSwap);
	close(listenningSocket);


	return EXIT_SUCCESS;
}
