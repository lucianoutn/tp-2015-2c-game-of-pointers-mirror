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
#include <SharedLibs/comun.h>

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

// STRUCT TABLA PARA CADA PROCESO QUE LLEGA //
typedef struct
{
   int pag; // Contiene el numero de pagina del proceso
   char * direccion_fisica; //Contiene la direccion de memoria de la pagina que se esta referenciando
}process_pag;
// ------------------------------------//

// DECLARACIONES GLOBALES PARA USAR FUNCIONES DE CONDICION EN LISTFIND
int numero_de_pid;
int numero_pagina;
// ESTRUCTURA QUE CONTIENE EL CONTEXTO DE LA MEMORIA
contexto miContexto;

sem_t * semConexion;



/* DECLARACIONES GLOBAL PARA CONOCER EL ESTADO DE LOS FRAMES DESDE CALQUIER LADO Y NO ANDAR PASANDO LA LISTA
 * DE FUNCION EN FUNCION
 */
t_list * listaFramesMemR; // LISTA DE FRAMES OCUPADOS
t_list * listaFramesHuecosMemR; // LISTA DE FRAMES VACIOS
// FUNCION QUE OBTIENE EL CONTEXTO DE LA MEMORIA Y LO GUARDA EN "MICONTEXTO"
void traigoContexto();
/* RESERVO UNA CANTIDAD DE MEMORIA PASADA EN BYTES POR PARAMATRO
 * SE USA PARA RESERVAR EL TAMAÑO DE LA MEMORIA REAL
 */
char * reservarMemoria(int);
// SE SOBREENTIENDE LA FUNCION
void liberarMemoria(char *);

// char* crear_tlb();

/* SE MANDA LA INSTRUCCION RECIBIDA Y SE HACE UN CASE DEL TIPO DE EJECUCION PARA SABER
   LAS ESTRUCTURAS A CREAR, QUE MANDAR AL SWAP, QUE RECIBIR, ETC... */
void ejecutoInstruccion(t_header *, char*,char*, t_list*, t_list*, int, int);

// FUNCIONES PARA USAR COMO FUNCION CONDICION EN LISTFIND O LISTFILTER POR EJEMPLO
bool elNodoTienePidIgualA(int *);
bool numeroDePaginaIgualA(int *);

//void iniciarEnCache(t_header, char*, char*); // INICIAR PROCESO EN TABLA TLB CON DIRECCION DE CACHE

/* AGREGO UN FRAME A LA LISTA DE FRAMES, SACO UNO DE LA LISTA DE FRAMES HUECOS. ESCRIBO EL FRAME CON LA
 	PAGINA RECIBIDA DEL SWAP */
void lectura(t_header *, t_list*, char*, char*, t_list *);

// SOLO CONEXION //
int meConectoAlSwap();
// ENVIO DE HEADER RETORNA UN 1 EN CASO DE EXITO Y 0 SI HUBO ALGUN ERROR EN SWAP PARA RECIBIR
int envioAlSwap(t_header *, int, char *);
// PREGUNTO SI LA TLB ESTA LLENA, DEVUELVE TRUE SI ESTA LLENA
bool tlbLlena(t_list *);
// FUNCION PARA FINALIZAR PROCESOS
void matarProceso(t_header *, t_list*);
/* ME DEVUELVE NULL SI LA PAGINA ESTA EN SWAP Y SINO ME DEVUELVE LA DIRECCION DEL MARCO */
//direccionDePag();

int buscarEnMemReal(t_list*,t_list *, t_header *, int,int, char *);

int buscarEnCache(int , t_list * );

/*
 * Unicamente crea las tablas para manejarlo
 * no le asigna marcos a las paginas
 */
void iniciarProceso();

/* PASAR A MANEJOLISTAS */
process_pag * pag_proc_create(int, char*);

static void pag_proc_destroy(process_pag *);
/* ---------------------*/

#endif /* SRC_FUNCMEMORY_H_ */

