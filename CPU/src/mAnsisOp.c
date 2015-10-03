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
	while(c!='\n' && c!=EOF) //agrego EOF sino no sale nunca salvo q despues del finalizar hayan hecho un ENTER
	{
		buffer[I]=c;
		I++;
		buffer= (char*)realloc(buffer, (I+1)*sizeof(char)); //solicito mas espacio en la memoria para buffer
		c=fgetc(fd);
	}
	buffer[I]='\0';
	return buffer;
}

char** leermCod(const char* ruta, int num)
{
	//El archivo tiene q estar en la ruta blahblah/CPU/archivo.algo
	FILE *fd = fopen(ruta,"r"); //abro el archivo mCod
	if(fd == NULL) printf("Error al abrir el mCod. Path: %s.\n",ruta);
	char **instruccion= (char**)malloc(sizeof(char*)); //solicito espacio en la memoria para el buffer
	num=0;
	while(!feof(fd))
	{
		instruccion[num]= (char*)malloc(sizeof(leerInstruccion(fd))); //solicito espacio en la memoria para la instruccion
		instruccion[num]= leerInstruccion(fd); //leo la instruccion
		if(strcmp(instruccion[num],"fin")==0) //para que no solicite mas espacio en la memoria
		{
			break;
		}
		num+=1;
		instruccion= (char**)realloc(instruccion, (num+1)*sizeof(char*)); //solicito mas espacio en la memoria para buffer
	}
	return instruccion;
}

