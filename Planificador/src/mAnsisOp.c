/*
 * mAnsisOp.c
 *
 *  Created on: 23/9/2015
 *      Author: utnso
 */
#include "mAnsisOp.h"

char* leerInstruccion(FILE *fd)
{
	char *buffer = (char*)malloc(sizeof(char)); //solicito espacio en la memoria para el buffer
	char c;
	int I=0;
	c= fgetc(fd); //leo el primer caracter de la instruccion
	if(c==EOF)
	{
		return "fin";
	}
	while(c!='\n')
	{
		buffer[I]=c;
		I++;
		buffer= (char*)realloc(buffer, (I+1)*sizeof(char)); //solicito mas espacio en la memoria para buffer
		c=fgetc(fd);
	}

	return buffer;
}

char** leermCod(const char* ruta, int num)
{
	FILE *fd = fopen(ruta,"r"); //abro el archivo mCod
	char **instruccion= (char**)malloc(sizeof(char*)); //solicito espacio en la memoria para el buffer
	int I=0;
	num=0;
	while(!feof(fd))
	{
		instruccion[num]= (char*)malloc(sizeof(leerInstruccion(fd))); //solicito espacio en la memoria para la instruccion
		instruccion[num]= leerInstruccion(fd); //leo la instruccion
		while(I!=strlen(instruccion[num])) //imprimo en pantalla (para ver como funciona dps lo sacamos
		{
			printf ("%c", instruccion[num][I]);
			I++;
		}
		printf ("\n");
		I=0;
		if(strcmp(instruccion[num],"fin")==0) //para que no solicite mas espacio en la memoria
		{
			break;
		}
		I++;
		instruccion= (char**)realloc(instruccion, (I+1)*sizeof(char*)); //solicito mas espacio en la memoria para buffer
	}

	return instruccion;
}

