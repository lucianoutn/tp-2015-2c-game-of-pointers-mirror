/*
 * CPU.h
 *
 *  Created on: 30/9/2015
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include "SharedLibs/libreriaCliente.h" //SharedLibs/Debug
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <SharedLibs/manejoListas.h>
#include <commons/config.h>
#include <semaphore.h>
#include "mAnsisOp.h"

typedef int pid;
pid PID_actual;
typedef bool flag;


#endif /* CPU_H_ */
