/*
 * sockets.h
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */

#ifndef SHAREDLIBS_SOCKETS_H_
#define SHAREDLIBS_SOCKETS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar


void conexionAlServer(int * serverSocket, char * puerto);
void conexionAlCliente(int * listenningSocket, int * socketCliente, char * puerto);


#endif /* SHAREDLIBS_SOCKETS_H_ */
