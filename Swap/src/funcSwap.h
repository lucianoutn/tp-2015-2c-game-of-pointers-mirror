/*
 * funcSwap.h
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */

#ifndef FUNCSWAP_H_
#define FUNCSWAP_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <SharedLibs/manejoListas.h>
#include <unistd.h>
#include <SharedLibs/comun.h>

typedef struct{
	char * puerto;
	char * nombre;
	int cant_paginas;
	int tam_pagina;
	int retardo_swap;
	int retardo_compac;
}t_contexto;

typedef struct{
	int status;
	char* contenido;
}t_devuelvo;
//Declaro variables globales
t_contexto * contexto;
FILE* archivo;
t_list * lista_paginas;
t_list * lista_huecos;
t_header * global;


FILE* crearParticion();

void cerrarParticion();

void traigoContexto();

char * numero_de_pagina(char *);

void analizoPaquete(t_header *, int );

void leerSwap(t_header*,char *);

int escribirSwap(t_header*, int);

int inicializarProc(t_header*);

int finalizarProc(t_header*);

void rellenarParticion(int, int);

t_hueco* buscarHueco(int);

bool numeroDePid(int *);

#endif /* FUNCSWAP_H_ */
