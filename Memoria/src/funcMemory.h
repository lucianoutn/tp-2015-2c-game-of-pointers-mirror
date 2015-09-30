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

contexto miContexto;
int numero_de_pid;
void traigoContexto();
char * reservarMemoria(int);
void liberarMemoria(char *);
char* crear_tlb();
void ejecutoInstruccionEnCache(t_header, char *, t_list *);
void ejecutoInstruccionEnMem(t_header, char *, t_list * );
void finalizarProceso(t_header);
bool elNodoTienePidIgualA(int *);
void iniciarEnCache(t_header, char*, char*); // INICIAR PROCESO EN TABLA TLB CON DIRECCION DE CACHE
void iniciarEnMemReal(t_header, char*, char*);
void meConectoAlSwap(t_header, char*);


#endif /* SRC_FUNCMEMORY_H_ */
