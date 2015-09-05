/*
 ============================================================================
 Name        : Planificador.c
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


int main() {
	puts("!!!Planificador!!!"); /* prints !!!Planificador!!! */

	//Configuracion del socket
	struct addrinfo hints; //estructura que almacena los datos de conexion del Planificador
	struct addrinfo *serverInfo; //estructura que almacena los datos de conexion de la CPU

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion

	//se crea un nuevo socket que se utilizara para la conexion con el CPU
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
		if (L == -1)
			perror("LISTEN");
		puts("LISTEN EJECUTANDOSE");


	//Estructura que tendra los datos de la conexion del cliente
	conexiones.socket_escucha = listenningSocket;
	conexiones.tamanio_direccion = sizeof(conexiones.direccion);
	pthread_t hilo_escuchas;
	if(pthread_create(&hilo_escuchas,NULL,escuchar,&conexiones)<0)
		puts("Error HILO ESCUCHAS!");

	puts("ESPERANDO CONEXIONES....\n");
	while(conexiones.CPUS[0] == 0){
	};

	while(1)
	{

		int socket_instrucciones;
		puts("Elija CPU:\n");
		int j = 0;
		while ( j < 5) {
			printf("CPU n°:%d, puerto: %d\n",j,conexiones.CPUS[j]);
			j++;
		}
		int caracter;
		scanf("%d", &caracter);
		switch (caracter) {
			case 0: socket_instrucciones = conexiones.CPUS[0];
				break;
			case 1: socket_instrucciones = conexiones.CPUS[1];
				break;
			case 2: socket_instrucciones = conexiones.CPUS[2];
				break;
			case 3: socket_instrucciones = conexiones.CPUS[3];
				break;
			case 4: socket_instrucciones = conexiones.CPUS[4];
				break;
			case 5: socket_instrucciones = conexiones.CPUS[5];
				break;
			default: puts("CPU NO VALIDA!");
				break;
		}
		int enviar = 1;
		char message[PACKAGESIZE];

		printf("Ya puede enviar instrucciones.\nEscriba 'correr' para enviar una señal al CPU\n'exit' para cambiar de CPU\n");

		while(enviar){
		fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
		if (!strcmp(message,"correr\n")) send(socket_instrucciones, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
		}

	}

		close(conexiones.socket_escucha);
		close(listenningSocket);

	return EXIT_SUCCESS;
}

