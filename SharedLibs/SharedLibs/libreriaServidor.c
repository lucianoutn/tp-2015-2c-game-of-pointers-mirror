/*
 * libreriaServidor.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#include "libreriaServidor.h"

//Funcion encargada de acceptar nuevas peticiones de conexion
void *escuchar (struct Conexiones* conexion){
	int i =0;

	while( i<MAX_CPUS ) //limite temporal de 1 CPUS conectada
	{
		//guarda las nuevas conexiones para acceder a ellas desde cualquier parte del codigo
		conexion->CPU[i] = accept(conexion->socket_escucha, (struct sockaddr *) &conexion->direccion, &conexion->tamanio_direccion);
		if(conexion->CPU[i]==-1)
		{
			perror("ACCEPT");	//control error
		}
		puts("NUEVO HILO ESCUCHA!\n");
		i++;
	}

	return NULL;
}


//Funcion que permite configurar la conexion y crear el socket. Devuelve el descriptor del socket nuevo.
int crearServer(const char *PUERTO)
{
	struct addrinfo hints; //estructura que almacena los datos de conexion
	struct addrinfo *serverInfo; //estructura que almacena los datos de conexion

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion

	//se crea un nuevo socket que se utilizara para la conexion con el cliente
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

	return listenningSocket;
}
