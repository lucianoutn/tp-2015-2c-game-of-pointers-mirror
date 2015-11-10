/*
 * libreriaServidor.h
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#ifndef SRC_LIBRERIASERVIDOR_H_
#define SRC_LIBRERIASERVIDOR_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "comun.h"

sem_t semEsperaCPU;

int crearServer ( const char *PUERTO);

#endif /* SRC_LIBRERIASERVIDOR_H_ */
