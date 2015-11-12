/*
 * consola.h
 *
 *  Created on: 5/10/2015
 *      Author: utnso
 */

#ifndef SRC_CONSOLA_H_
#define SRC_CONSOLA_H_

//#include "libreriaPlanificador.h"
//#include <semaphore.h>
//#include "SharedLibs/comun.h"

/********************************************************
 * LIBRERIA DEDICADA UNICAMENTE A LA CONSOLA
 ********************************************************/

#include <ctype.h>
#include "commons/string.h"

enum Commands
{
	// esto define los comandos en la forma en que los vamos a manipular internamente
	// son enteros que representan a un estado, que no sean solo números ayuda a la lectura
	// pero es solo de uso interno
	// si se ingresa un nuevo comando o se modifica, se debe modificar también su representación
	// del lado del usuario, en la lista de arriba.
	ayuda = 0,
	correr,	// tendria q ser correr PATH directamente acá
	formatearmcod,
	finalizar,
	ps,
	cpu,
	salir,
	enter
};

void consola (void*);

int leeComando(void);

int orden; //var q permite verificar q orden se ingreso x consola

#endif /* SRC_CONSOLA_H_ */
