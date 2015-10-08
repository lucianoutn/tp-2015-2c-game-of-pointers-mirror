/*
 * comun.h
 *
 *  Created on: 1/10/2015
 *      Author: utnso
 */

#ifndef SHAREDLIBS_COMUN_H_
#define SHAREDLIBS_COMUN_H_

#include <commons/log.h>
#include <semaphore.h>
/*
 * Utilizamos esta libreria para poner todas las declaraciones
 * de variables que van a ser globales a TODOS los procesos.
 * Por ejemplo, el archivo de loggeo tienen que poder verlo todos.
 * Al igual que los semaforos que vamos a utilizar para sincronizacion
 * A.
 */
#include <unistd.h>

t_log* logger;
sem_t semProduccionMsjs;

char* cwd();

void creoLogger(int pantalla);

#endif /* SHAREDLIBS_COMUN_H_ */
