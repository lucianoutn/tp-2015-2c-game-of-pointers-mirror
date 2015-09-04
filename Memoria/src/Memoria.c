/*
 ============================================================================
 Name        : Memoria.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Memoria
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
#define PUERTO "9000" //este es el puerto de swap??
#define BACKLOG 10
#define PACKAGESIZE 1024


int main(void) {
	puts("!!!Memoria!!!"); /* prints !!!Memoria!!! */

	//Configuracion del socket
		struct addrinfo hints; //estructura que almacena los datos de conexion de la Memoria
		struct addrinfo *serverInfo; //estructura que almacena los datos de conexion del Swap

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
		hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

		getaddrinfo(IP, PUERTO, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	//se crea un nuevo socket que se utilizara para la conexion con el swap
		int memSocket; //descriptor del socket de la memoria
		memSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	//se comprueba que el socket se creo correctamente
		if(memSocket==-1)
			perror ("SOCKET");

	//Conexion al servidor Swap
		int C = connect(memSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
		if (C==-1)
			perror ("CONNECT");
		else
			printf ("Conexion con el Swap lograda\n");

		freeaddrinfo(serverInfo);

	//Envio de instrucciones

		int enviar = 1;
		char message[PACKAGESIZE];
		if (C!=1)
			printf("Conectado al Swap en el Socket=%d. Ya puede enviar instrucciones. Escriba 'exit' para finalizar\n",memSocket);

		while(enviar){
			fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
			if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
			if (enviar) send(memSocket, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
		}



		return EXIT_SUCCESS;
}
