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
	struct addrinfo hints; //estructura que almacena los datos de conexion
	struct addrinfo *serverInfo; //estructura que almacena los datos de conexion
	struct sockaddr_in myaddr;

	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(atoi(PUERTO));
	memset(&(myaddr.sin_zero), '\0', 8); // SE TIENE QUE RELLENAR DE 0, NO SE SI HACE FALTA (explica en beej)

	/*memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = INADDR_ANY;// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	hints.ai_protocol = 0;*/

	//getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion

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

	freeaddrinfo(serverInfo);

	return listenningSocket;
}
