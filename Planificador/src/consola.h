/*
 * consola.h
 *
 *  Created on: 5/10/2015
 *      Author: utnso
 */

#ifndef SRC_CONSOLA_H_
#define SRC_CONSOLA_H_

#include <semaphore.h>
#include "SharedLibs/comun.h"

/********************************************************
 * LIBRERIA DEDICADA UNICAMENTE A LA CONSOLA
 ********************************************************/

const char* allCommands[] =
{
	// esto define las palabras de los comandos aceptados
	// lo que el usuario ingrese va a ser comparado con esto
	// TODOS LOS COMANDOS deben estar en minúscula para que lo reconozca bien sin importar como lo ingrese el usuario
	"ayuda",
	"iniciar", // tendria q ser correr PATH directamente acá
	"finalizar PID",
	"ps",
	"cpu",
	"salir"
};

enum Commands
{
	// esto define los comandos en la forma en que los vamos a manipular internamente
	// son enteros que representan a un estado, que no sean solo números ayuda a la lectura
	// pero es solo de uso interno
	// si se ingresa un nuevo comando o se modifica, se debe modificar también su representación
	// del lado del usuario, en la lista de arriba.
	ayuda = 0,
	iniciar,	// tendria q ser correr PATH directamente acá
	finalizar,
	ps,
	cpu,
	salir,
	enter
};

int consola ();

int leeComando(void);


#endif /* SRC_CONSOLA_H_ */
