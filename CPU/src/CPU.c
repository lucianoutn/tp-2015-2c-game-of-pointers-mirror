/*
 ============================================================================
 Name        : CPU.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Planificador
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#define IP "127.0.0.1"
#define PUERTO "8080"
#define PACKAGESIZE 1024

//Inicio de funcion principal
int main()
{
	puts("!!!CPU!!"); /* prints !!!CPU!! */

	//Configuracion del socket
	struct addrinfo hints; //estructura que almacena los datos de conexion de la CPU
	struct addrinfo *serverInfo; //estructura que almacena los datos de conexion del Planificador

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	//se crea un nuevo socket que se utilizara para la conexion con el Planificador
	int cpuSocket; //descriptor del socket servidor
	cpuSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	//se comprueba que el socket se creo correctamente
	if(cpuSocket==-1)
		perror ("SOCKET");

	//Conexion al servidor Planificador
	int C = connect(cpuSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	if (C==-1){
		perror ("CONNECT");
		return -1;
	}else
		printf ("Conexion con el Planificador lograda\n");

	freeaddrinfo(serverInfo);

//Recepcion de instrucciones


	char package[PACKAGESIZE];
		int status = 1;		// Estructura que manjea el status de los recieve.

		printf("CPU conectada. Esperando instrucciones:\n");

		while (status != 0){
			status = recv(cpuSocket, (void*) package, PACKAGESIZE, 0);
			if (status != 0) printf("%s", package);

		}




	close(cpuSocket);



	return EXIT_SUCCESS;
}
