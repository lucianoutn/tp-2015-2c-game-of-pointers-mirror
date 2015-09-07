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
	int CPU[10];
} conexiones;

//Funcion encargada de acceptar nuevas peticiones de conexion
void* escuchar (struct Conexiones* conexion){
	int i =0;

	while( i<=5 ) //limite temporal de 5 CPUS conectadas
	{
		//guarda las nuevas conexiones para acceder a ellas desde cualquier parte del codigo
		conexion->CPU[i] = accept(conexion->socket_escucha, (struct sockaddr *) &conexion->direccion, &conexion->tamanio_direccion);
		if(conexion->CPU[i]==-1)
		{
			return -1;
		}
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


	//Estructura que tendra los datos de la conexion del cliente
	conexiones.socket_escucha = listenningSocket;
	conexiones.tamanio_direccion = sizeof(conexiones.direccion);
	pthread_t hilo_escuchas;
	if(pthread_create(&hilo_escuchas,NULL,escuchar,&conexiones)<0)
		perror("Error HILO ESCUCHAS!");

	puts("ESPERANDO CONEXIONES....\n");
	while(conexiones.CPU[0] == 0){
	};


	int socket_instrucciones, caracter, enviar;
	char message[PACKAGESIZE];

	while(strcmp(message,"salir\n") !=0)
	{
		puts("Elija CPU: ¡¡¡¡SOLO NUMEROS!!!\n");
		int j = 1;
		while ( j < 6) {
			printf("CPU n°:%d, puerto: %d\n",j,conexiones.CPU[j-1]);
			j++;
		}
		scanf("%d", &caracter);
		enviar =1;
		switch (caracter) {
			case 1:
				socket_instrucciones = conexiones.CPU[0];
				break;
			case 2:
				socket_instrucciones = conexiones.CPU[1];
				break;
			case 3: socket_instrucciones = conexiones.CPU[2];
				break;
			case 4:
				socket_instrucciones = conexiones.CPU[3];
				break;
			case 5:
				socket_instrucciones = conexiones.CPU[4];
				break;
			default:
				{puts("CPU NO VALIDA!"); enviar=0; caracter=0;};
				break;
		}

		printf("Ya puede enviar instrucciones.\nEscriba 'correr programa' para enviar una señal al CPU\n'cpu' para cambiar de CPU\n'salir' para cerrar los procesos\n");
		while(enviar){
		fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message,"cpu\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
		if (!strcmp(message,"correr programa\n")) send(socket_instrucciones, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
		if (!strcmp(message,"salir\n")){ send(socket_instrucciones, message, strlen(message) + 1, 0); break;};
		}

	}

		close(conexiones.socket_escucha);
		close(listenningSocket);

	return EXIT_SUCCESS;
}

