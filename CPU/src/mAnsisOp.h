/*
 * mAnsisOp.h
 *
 *  Created on: 23/9/2015
 *      Author: utnso
 */

#ifndef SRC_MANSISOP_H_
#define SRC_MANSISOP_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*Funcion que lee el archivo indicado en el parametro ruta y retorna un vector de punteros, en donde
 * cada puntero es una instruccion del archivo. El parametro num indica la cantidad de instrucciones
 * que tiene el archivo.
*/
char** leermCod(const char* ruta, int num);

//Funcion que permite leer instruccion por instruccion en un archivo
char* leerInstruccion(FILE *fd);

#endif /* SRC_MANSISOP_H_ */
