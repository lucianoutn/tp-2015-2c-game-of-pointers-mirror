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
#include <pthread.h>

#define IP "127.0.0.1"
#define PUERTOSWAP "9000" //este es el puerto de swap
#define PUERTOCPU "8090"
#define BACKLOG 10
#define PACKAGESIZE 1024

//struct para conexiones
struct Conexiones {
	int socket_escucha;
	struct sockaddr_in direccion;
	socklen_t tamanio_direccion;
	int CPUS[10];
} conexiones;

//Funcion encargada de acceptar nuevas peticiones de conexion
void *escuchar (struct Conexiones *conexion){
	int i =0;

	while( i<=5 ) //limite temporal de 5 CPUS conectadas
	{
		//guarda las nuevas conexiones para acceder a ellas desde cualquier parte del codigo
		conexion->CPUS[i] = accept(conexion->socket_escucha, (struct sockaddr *) &conexion->direccion, &conexion->tamanio_direccion);
		puts("NUEVO HILO ESCUCHA!\n");
		i++;
	}
	return NULL;
}


int main(void) {
	puts("!!!Memoria!!!"); /* prints !!!Memoria!!! */

	//Configuracion del socket
	struct addrinfo hints; //estructura que almacena los datos de conexion de la Memoria
	struct addrinfo *serverInfo; //estructura que almacena los datos de conexion del Swap

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTOSWAP, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

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

	//freeaddrinfo(serverInfo);

	getaddrinfo(IP, PUERTOCPU, &hints, &serverInfo);

	//inicio server
	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	//se comprueba que el socket se creo correctamente
	if (listenningSocket == -1)
		perror("SOCKET");

	//se comprueba que la asociacion fue exitosa
	int B = bind(listenningSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
			if (B == -1)
			perror("BIND");

	freeaddrinfo(serverInfo);


	//funcion que permite al programa ponerse a la espera de nuevas conexiones
		int L = listen(listenningSocket, BACKLOG);
		if (L==-1)
			perror("LISTEN");


	//Estructura que tendra los datos de la conexion del cliente
	conexiones.socket_escucha = listenningSocket;
	conexiones.tamanio_direccion = sizeof(conexiones.direccion);
	pthread_t hilo_escuchas;
	if(pthread_create(&hilo_escuchas, NULL, escuchar,&conexiones)<0)
		puts("Error HILO ESCUCHAS!");

	puts("ESPERANDO CPU....\n");
	while(conexiones.CPUS[0]==0);

	//fin server

	char message[PACKAGESIZE] = "lalala";
	int status;
	printf("CPU conectada. Esperando instrucciones:\n");
	//Envio de instrucciones
	while(strcmp(message,"salir\n") !=0)
	{
		status = recv(conexiones.CPUS[0], (void*) message, PACKAGESIZE, 0);
		if (status != 0){
			printf("RECIBIDO! =D\n%s", message);
			send(memSocket, message, strlen(message) + 1, 0);
		}
		else{
			puts("conexion perdida! =(");
			break;
		}
	}


	return EXIT_SUCCESS;
}
