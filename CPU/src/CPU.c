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
#define PUERTOPLANIFICADOR "8080"
#define PUERTOMEMORIA "8090"
#define PACKAGESIZE 1024

//Inicio de funcion principal
int main()
{
	puts("!!!CPU!!"); /* prints !!!CPU!! */

	//Configuracion del socket
	struct addrinfo hints_planificador, hints_memoria; //estructura que almacena los datos de conexion de la CPU
	struct addrinfo *serverInfo_planificador, *serverInfo_memoria; //estructura que almacena los datos de conexion del Planificador

	memset(&hints_planificador, 0, sizeof(hints_planificador));
	hints_planificador.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints_planificador.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	serverInfo_memoria = serverInfo_planificador;
	hints_memoria = hints_planificador;
	getaddrinfo(IP, PUERTOPLANIFICADOR, &hints_planificador, &serverInfo_planificador);	// Carga en serverInfo los datos de la conexion

	//se crea un nuevo socket que se utilizara para la conexion con el Planificador
	int socketPlanificador, socketMemoria; //descriptor del socket servidor
	socketPlanificador = socket(serverInfo_planificador->ai_family, serverInfo_planificador->ai_socktype, serverInfo_planificador->ai_protocol);
	//se comprueba que el socket se creo correctamente
	if(socketPlanificador==-1)
		perror ("SOCKET PLANIFICADOR!");

	getaddrinfo(IP,PUERTOMEMORIA,&hints_memoria,&serverInfo_memoria);

	socketMemoria = socket(serverInfo_memoria->ai_family, serverInfo_memoria->ai_socktype, serverInfo_memoria->ai_protocol);
	if(socketMemoria==-1)
		perror("SOCKET MEMORIA!");

	//Conexion al servidor Planificador
	int C = connect(socketPlanificador, serverInfo_planificador->ai_addr, serverInfo_planificador->ai_addrlen);
	if (C==-1){
		perror ("CONNECT");
		return -1;
	}
	else
		printf ("Conexion con el Planificador lograda\n");

	int C2 = connect(socketMemoria, serverInfo_memoria->ai_addr, serverInfo_memoria->ai_addrlen);
	if (C2==-1){
		perror ("CONNECT");
		return -1;
	}
	else
		printf ("Conexion con la Memoria lograda\n");


	freeaddrinfo(serverInfo_planificador);
	freeaddrinfo(serverInfo_memoria);

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



	return EXIT_SUCCESS;
}
