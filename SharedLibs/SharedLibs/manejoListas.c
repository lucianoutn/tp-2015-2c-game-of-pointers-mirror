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

/* --------- MARCOS OCUPADOS Y MARCOS HUECOS --------------------*/
t_marco * marco_create (char * direccion_inicio, int num_marco)
{
 t_marco * frame = malloc(sizeof(t_marco));
 frame->numero_marco = num_marco;
 frame->direccion_inicio = direccion_inicio;
 return frame;
}

static void marco_destroy(t_marco * self)
{
 free(self);
}

t_marco_hueco * marco_hueco_create (char * direccion_inicio, int num_marco)
{
 t_marco_hueco * frame = malloc(sizeof(t_marco));
 frame->numero_marco = num_marco;
 frame->direccion_inicio = direccion_inicio;
 return frame;
}

static void marco_hueco_destroy(t_marco_hueco * self)
{
 free(self);
}
/* ---------------------------------------------------------*/

/* -------------- TABLA DE TABLAS DE PROCESOS --------------*/
t_tabla_adm * tabla_adm_create (int pid, t_list * lista_proceso)
{
 t_tabla_adm * adm_proc = malloc(sizeof(t_tabla_adm));
 adm_proc->direc_tabla_proc = lista_proceso;
 adm_proc->pid = pid;
 return adm_proc;
}

static void tabla_adm_destroy(t_tabla_adm * self)
{
 free(self);
}
/* ---------------------------------------------------------*/

/* ---------------------- MANEJO TLB ----------------------*/
t_tlb * reg_tlb_create (int pid, int pagina, char * direccion_fisica)
{
 t_tlb * prueba = malloc(sizeof(t_tlb));
 prueba->pid = pid;
 prueba->pagina = pagina;
 prueba->direccion_fisica = direccion_fisica;
 return prueba;
}

static void reg_tlb_destroy(t_tlb * self)
{
 free(self);
}
/* ---------------------------------------------------------*/

/* ---------------- SWAP ---------------------*/
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
/* ---------------------------------------------*/

/* ----------------------MEMORIA -----------------*/
t_list * crearListaFrames()
{
	 t_list * lista_frames = list_create();
	 return lista_frames;
}

t_list * crearListaHuecosFrames()
{
	 t_list * lista_huecos_frames = list_create();
	 return lista_huecos_frames;
}

t_list * crearListaTlb()
{
 t_list * lista_tlb = list_create();
 return lista_tlb;
}

t_list * crearListaAdm()
{
	 t_list * lista_administracion = list_create();
	 return lista_administracion;
}
