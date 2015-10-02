/*
 * planificador.h
 *
 *  Created on: 17/9/2015
 *      Author: utnso
 */

#ifndef SRC_PLANIFICADOR_H_
#define SRC_PLANIFICADOR_H_

#include "SharedLibs/libreriaServidor.h" //SharedLibs/Debug
#include "libreriaPlanificador.h"
#include <commons/collections/list.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10
#define PACKAGESIZE 1024
#define MAX_CPUS 5
//hola

#endif /* SRC_PLANIFICADOR_H_ */
