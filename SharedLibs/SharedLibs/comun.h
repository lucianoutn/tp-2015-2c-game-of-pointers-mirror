/*
 * comun.h
 *
 *  Created on: 1/10/2015
 *      Author: utnso
 */

#ifndef SHAREDLIBS_COMUN_H_
#define SHAREDLIBS_COMUN_H_

#include <commons/log.h>
/*
 * Utilizamos esta libreria para poner todas las declaraciones
 * de variables que van a ser globales a TODOS los procesos.
 * Por ejemplo, el archivo de loggeo tienen que poder verlo todos.
 * Al igual que los semaforos que vamos a utilizar para sincronizacion
 * A.
 */
t_log* logger;

void creoLogger();
#endif /* SHAREDLIBS_COMUN_H_ */