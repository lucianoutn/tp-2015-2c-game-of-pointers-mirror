/*
 ============================================================================
 Name        : Planificador.c
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
#include <pthread.h>

#define IP "127.0.0.1"
#define PUERTO "8080"
#define BACKLOG 10
#define PACKAGESIZE 1024


struct param{
 int socket;
 int n;
};

void escuchar (struct param *parametros) {


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

	//Hilo para escucha

/*
	struct param param1 = {0,0};
	pthread_t hiloListen;
	pthread_create (&hiloListen, NULL, (void*)escuchar, (void*)&param1);
*/



	//Estructura que tendra los datos de la conexion del cliente
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int socketPlanificador = accept(listenningSocket, (struct sockaddr *) &addr,
			&addrlen);
	printf("Conexion recibida\n");

	//Envio de instrucciones

	int enviar = 1;
		char message[PACKAGESIZE];

		printf("Conectado al CPU. Ya puede enviar instrucciones. Escriba 'exit' para finalizar\n");

		while(enviar){
			fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
			if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
			if (enviar) send(socketPlanificador, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
		}

		close(socketPlanificador);
		close(listenningSocket);

	return EXIT_SUCCESS;
}
