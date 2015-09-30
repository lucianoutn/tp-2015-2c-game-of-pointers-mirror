/*
 * manejoListas.h
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */

#ifndef SHAREDLIBS_MANEJOLISTAS_H_
#define SHAREDLIBS_MANEJOLISTAS_H_


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/collections/list.h>

typedef struct {
 int pid;
 int inicio;
 int paginas;
} t_pag;

typedef struct {
 int inicio;
 int paginas;
} t_hueco;

// STRUCT CPU -> MEMORY -> SWAP /
typedef struct
{
 int type_ejecution;
 int PID;
 int pagina_proceso;
 int tamanio_msj;
} t_header;
/* -------------------------------*/

// STRUCT TLB /
typedef struct
{
 int PID;
 int marco;
 char * direc_mem;
} ttlb;
/* -------------------*/

// STRUCT CACHE /
typedef struct
{
 int PID;
 int marco;
 char * direc_mem;
} t_cache;
/* -------------------*/

// STRUCT TABLA MEMORIA REAL /
typedef struct
{
 int PID;
 int marco;
 char * direc_mem;
} t_tabla_mr;
/* -------------------*/


t_pag * pag_create(int,int, int);

void pag_destroy(t_pag *);

t_hueco * hueco_create(int, int);

void hueco_destroy(t_hueco *);

ttlb * input_create (int pid, int marco, char * direc_mem);

static void input_destroy(ttlb * self);

t_tabla_mr * in_create(int,int, char*);

static void in_destroy(t_tabla_mr *);

t_list * crearListaPaginas();

t_list * crearListaHuecos(int);

t_list * crearListaTlb();

t_list * crearListaCache();

t_list * crearListaTMemReal();

t_list * crearListaMemReal();


#endif /* SHAREDLIBS_MANEJOLISTAS_H_ */
