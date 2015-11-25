/*
 * libreriaServidor.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#include "libreriaServidor.h"

//Funcion que permite configurar la conexion y crear el socket. Devuelve el descriptor del socket nuevo.
int crearServer(const char *PUERTO)
{
	int yes=1;
	struct sockaddr_in myaddr;

	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(atoi(PUERTO));
	memset(&(myaddr.sin_zero), '\0', 8);

	//se crea un nuevo socket que se utilizara para la conexion con el cliente
	int listenningSocket;
	listenningSocket = socket(AF_INET, SOCK_STREAM, 0);
	//se comprueba que el socket se creo correctamente
	if (listenningSocket == -1)
		perror("SOCKET");

	if (setsockopt(listenningSocket, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(1);
	}
	//se comprueba que la asociacion fue exitosa
	int B = bind(listenningSocket,(struct sockaddr *)&myaddr, sizeof(myaddr));
	if (B == -1) {
		perror("BIND Y LA P@%#\n");
		abort();
	}

	return listenningSocket;
}
