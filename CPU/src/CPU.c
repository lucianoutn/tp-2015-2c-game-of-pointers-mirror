/*
 ============================================================================
 Name        : CPU.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - CPU
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
#define PUERTO "8080"
#define BACKLOG 10

int main(void)
{
	puts("!!!CPU!!!"); /* prints !!!CPU!!! */

	//Configuracion del socket
	struct addrinfo hints; //estructura que almacena los datos de conexion de la CPU
	struct addrinfo *serverInfo; //estructura que almacena los datos de conexion del planificador

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo);  // Carga en serverInfo los datos de la conexion

	//se crea un nuevo socket que se utilizara para la conecxion con el Planificador
	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	//se comprueba que el socket se creo correctamente
	if (listenningSocket==-1)
		perror ("SOCKET");

	//se comprueba que la asociacion fue exitosa
	int B = bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	if (B==-1)
	    perror("BIND");

	freeaddrinfo(serverInfo);

	//funcion que permite al programa ponerse a la espera de nuevas conexiones
	int L= listen (listenningSocket, BACKLOG);
	if (L==-1)
	    perror ("LISTEN");

	//Estructura que tendra los datos de la conexion del cliente
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int socketPlanificador = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
	printf ("Conexion recivida\n");

	return EXIT_SUCCESS;
}
