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

//prueba commit sadasdaadaadad
//prueba para hilos
/*se pueden conectar varios clientes y envia mensajes dependiendo cm se le de la gana al hilo
 * por ahora lo dejo asi, porque con 1 solo CPU anda bien, y mejor terminar el resto antes de
 * seguir con esto. Tambien depende de como mandemos las instrucciones y demas de la planificacion
 * que todavia no hicimos del TP para saber como tendria que mandar los mensajes.
 */


void *multiplesConexiones (void *socket_desc) {

	int socket_instrucciones = *(int *)socket_desc;

	//Envio de instrucciones

	int enviar = 1;
	char message[PACKAGESIZE];

	printf("Conectado al CPU en el Socket=%d. Ya puede enviar instrucciones. Escriba 'exit' para finalizar\n",socket_instrucciones);

	while(enviar){
		fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
		if (enviar) send(socket_instrucciones, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
	}


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
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int socketPlanificador, *nuevo_socket;

	//Hilo para escucha (prueba)

	while( (socketPlanificador = accept(listenningSocket, (struct sockaddr *) &addr,	&addrlen) ) )
	{
		printf("Conexion aceptada Socket=&d\n",socketPlanificador);

		pthread_t hilo;
		nuevo_socket=malloc(1);
		*nuevo_socket=socketPlanificador;

		if(pthread_create(&hilo,NULL,multiplesConexiones,(void*)nuevo_socket)<0)
		{
			perror("No se puede crear el hilo");
			return 1;
		}

		puts("Nueva conexion asignada");
	}

		//free(nuevo_socket);

		close(socketPlanificador);
		close(listenningSocket);

	return EXIT_SUCCESS;
}
