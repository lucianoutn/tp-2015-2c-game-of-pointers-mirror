/*
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
#include <commons/config.h>
#include <SharedLibs/manejoListas.h>

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
   char * direccion_fisica; //Contiene la direccion de memoria de la pagina que se esta referenciando
}process_pag;
/* ------------------------------------*/



int numero_de_pid;
int numero_pagina;

contexto miContexto;
t_list * listaFramesMemR; // LISTA DE FRAMES OCUPADOS
t_list * listaFramesHuecosMemR; // LISTA DE FRAMES VACIOS
void traigoContexto();
char * reservarMemoria(int);
void liberarMemoria(char *);
char* crear_tlb();
void ejecutoInstruccion(t_header *, char*,char*, t_list*, t_list*);
bool elNodoTienePidIgualA(int *);
bool numeroDePaginaIgualA(int *);
void iniciarEnCache(t_header, char*, char*); // INICIAR PROCESO EN TABLA TLB CON DIRECCION DE CACHE
void iniciarEnMemReal(t_header *, t_list*, char*);
void meConectoAlSwap(t_header *, char*);
bool tlbLlena(t_list *);

/* PASAR A MANEJOLISTAS */
process_pag * pag_proc_create(int, char*);

static void pag_proc_destroy(process_pag *);
/* ---------------------*/

#endif /* SRC_FUNCMEMORY_H_ */
