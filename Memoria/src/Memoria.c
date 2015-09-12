/*
 ============================================================================
 Name        : Memoria.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Memoria
 ============================================================================
 */

#include "libreriaServidor.h"


#define BACKLOG 10
#define PACKAGESIZE 1024

const char *IP="127.0.0.1";
const char *PUERTOSWAP="9000";
const char *PUERTOMEM="8090";

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
	int memSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol); //descriptor del socket de la memoria
	//se comprueba que el socket se creo correctamente
	if(memSocket==-1)
		perror ("SOCKET");

	//Conexion al servidor Swap
	int C = connect(memSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	if (C==-1)
		perror ("CONNECT");
	else
		printf ("Conexion con el Swap lograda\n");


	//inicio server
	int CPUSocket= crearServer(IP, PUERTOMEM);

	//funcion que permite al programa ponerse a la espera de nuevas conexiones
	int L = listen(CPUSocket, BACKLOG);
	if (L==-1)
		perror("LISTEN");


	//Estructura que tendra los datos de la conexion del cliente
	conexiones.socket_escucha = CPUSocket;
	conexiones.tamanio_direccion = sizeof(conexiones.direccion);
	pthread_t hilo_escuchas;
	if( pthread_create(&hilo_escuchas, NULL, escuchar,&conexiones) < 0)
		puts("Error HILO ESCUCHAS!");

	puts("ESPERANDO CPU....\n");
	while(conexiones.CPU[0]==0);

	//fin server

	char message[PACKAGESIZE] = "lalala";
	int status;
	printf("CPU conectada. Esperando instrucciones:\n");
	//Envio de instrucciones
	while(strcmp(message,"salir\n") !=0)
	{
		status = recv(conexiones.CPU[0], (void*) message, PACKAGESIZE, 0);
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
