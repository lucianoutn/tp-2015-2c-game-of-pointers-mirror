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
	struct addrinfo hints_planificador, hints_memoria; //estructura que almacena los datos de conexion de la CPU
	struct addrinfo *serverInfo_planificador, *serverInfo_memoria; //estructura que almacena los datos de conexion del Planificador

//Funcion encargadar de conectar al cliente con los servidores

	int crearClientePlani(const char *IP, const char *PUERTOPLANIFICADOR);
	int crearClienteMem(const char *IP_MEMORIA, const char *PUERTOMEMORIA);

#endif /* LIBRERIACLIENTE_H_ */
