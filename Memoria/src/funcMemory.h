/*
 * funcMemory.h
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */

#ifndef SRC_FUNCMEMORY_H_
#define SRC_FUNCMEMORY_H_



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <SharedLibs/manejoListas.h>
#include <commons/config.h>

typedef struct{
 char * puertoServidor;
 char * puertoCliente;
 int maxMarcos;
 int cantidadMarcos;
 int tamanioMarco;
 int entradasTlb;
 char * tlbHabilitada;
 int retardoMemoria;
 char * algoritmoReemplazo;
} contexto;

/* STRUCT TABLA PARA CADA PROCESO QUE LLEGA */
typedef struct
{
	int pag; // Contiene el numero de pagina del proceso
	char * direc_mem; //Contiene la direccion de memoria de la pagina que se esta referenciando
} pag_proceso;
/* ------------------------------------*/

contexto miContexto;
int numero_de_pid;
int numero_pagina;
void traigoContexto();
char * reservarMemoria(int);
void liberarMemoria(char *);
char* crear_tlb();
void ejecutoInstruccion(t_header, char*, char*,char*, t_list*, t_list*);
void finalizarProceso(t_header);
bool elNodoTienePidIgualA(int *);
bool numeroDePaginaIgualA(int *);
void iniciarEnCache(t_header, char*, char*); // INICIAR PROCESO EN TABLA TLB CON DIRECCION DE CACHE
void iniciarEnMemReal(t_header, char*, char*);
void meConectoAlSwap(t_header, char*);
bool tlbLlena(t_list *);


#endif /* SRC_FUNCMEMORY_H_ */
