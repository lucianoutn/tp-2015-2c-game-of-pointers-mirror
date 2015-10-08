/*
 * libreriaCliente.c
 *
 *  Created on: 11/9/2015
 *      Author: utnso
 */


#include "libreriaCliente.h"

//Conexion al servidor
int crearCliente(const char *IP, const char *PUERTO){

	//configuraciones varias
	memset(&hints_server, 0, sizeof(hints_server));
	hints_server.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints_server.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTO, &hints_server, &serverInfo_server);	// Carga en serverInfo los datos de la conexion

	//se crea un nuevo socket que se utilizara para la conexion con el servidor
	//descriptor del socket servidor
	int socketServer = socket(serverInfo_server->ai_family, serverInfo_server->ai_socktype, serverInfo_server->ai_protocol);
	//se comprueba que el socket se creo correctamente
	if(socketServer==-1){
		return -1;
		perror ("SOCKET con servidor no se pudo crear!");
	}

	//conecta
	int C = connect(socketServer, serverInfo_server->ai_addr, serverInfo_server->ai_addrlen);
	if (C==-1){
		return -1;
		perror ("CONNECT");
	}
	else
		printf ("Conexion con el Servidor en la ip: %s lograda\n", IP);


	freeaddrinfo(serverInfo_server);

	return socketServer;
}
