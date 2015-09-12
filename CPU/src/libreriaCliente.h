/*
 * libreriaCliente.h
 *
 *  Created on: 11/9/2015
 *      Author: utnso
 */

#ifndef LIBRERIACLIENTE_H_
#define LIBRERIACLIENTE_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

//Configuracion del socket
	struct addrinfo hints_server; //estructura que almacena los datos de conexion ¿?. lucho
	struct addrinfo *serverInfo_server; //estructura que almacena los datos de conexion ¿?. lucho

//Funcion encargadar de conectar al cliente con los servidores


	int crearCliente(const char *IP, const char *PUERTO);


#endif /* LIBRERIACLIENTE_H_ */
