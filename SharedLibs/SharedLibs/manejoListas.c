/*
 * manejoListas.c
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */


#include "manejoListas.h"

t_pag * pag_create (int pid, int start, int paginas)
{
 t_pag * prueba = malloc(sizeof(t_pag));
 prueba->pid = pid;
 prueba->inicio = start;
 prueba->paginas = paginas;
 return prueba;
}

void pag_destroy(t_pag * self)
{
 free(self);
}

t_hueco * hueco_create ( int start, int paginas)
{
 t_hueco * prueba = malloc(sizeof(t_hueco));
 prueba->inicio = start;
 prueba->paginas = paginas;
 return prueba;
}

void hueco_destroy(t_hueco * self)
{
 free(self);
}

/* ---------------------- MANEJO TLB ----------------------*/
ttlb * input_create (int pid, int marco, char * direc_mem)
{
 ttlb * prueba = malloc(sizeof(ttlb));
 prueba->PID = pid;
 prueba->marco = marco;
 prueba->direc_mem = direc_mem;
 return prueba;
}

static void input_destroy(ttlb * self)
{
 free(self);
}
/* ---------------------------------------------------------*/

/* ---------------------- MANEJO TABLA MEMORIA REAL ----------------------*/
t_tabla_mr * in_create (int pid, int marco, char * direc_mem)
{
 ttlb * prueba = malloc(sizeof(ttlb));
 prueba->PID = pid;
 prueba->marco = marco;
 prueba->direc_mem = direc_mem;
 return prueba;
}

static void in_destroy(t_tabla_mr * self)
{
 free(self);
}
/* ---------------------------------------------------------*/

t_list * crearListaPaginas()
{
 t_list * lista_paginas= list_create();
 return lista_paginas;
}

t_list * crearListaHuecos(int cant)
{
 t_list * lista_huecos = list_create();
 int a = list_add(lista_huecos, hueco_create(0,cant));
 return lista_huecos;
}

t_list * crearListaTlb()
{
 t_list * lista_tlb = list_create();
 return lista_tlb;
}

t_list * crearListaTMemReal()
{
 t_list * lista_tmem_real = list_create();
 return lista_tmem_real;
}

t_list * crearListaMemReal()
{
 t_list * lista_mem_real = list_create();
 return lista_mem_real;
}

