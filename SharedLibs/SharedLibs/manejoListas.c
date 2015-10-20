/*
 /*
 * manejoListas.c
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */


#include "manejoListas.h"


/* MANEJO SWAP */
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
/* -----------------------------------------*/


/* ----------MANEJO DESDE CPU -> MEMORY -> SWAP-----*/

t_header * package_create (int tipo_ejecucion, int pid, int pagina, int tamanio)
{
	t_header * prueba = malloc(sizeof(t_header));
	prueba->PID = pid;
	prueba->pagina_proceso = pagina;
	prueba->tamanio_msj = tamanio;
	prueba->type_ejecution = tipo_ejecucion;
	return prueba;
}

static void package_destroy(t_header * self)
{
	free(self);
}
/* ---------------------------------------------*/


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

void reg_tlb_destroy(t_tlb * self)
{
 free(self);
}
/* ---------------------------------------------------------*/

/* --------------ENTRADAS A LA TABLA DE PROCESO ------------
pag_proceso * pag_proc_create (int pagina, char * direccion_fisica)
{
 pag_proceso * reg_pagina = malloc(sizeof(pag_proceso));
 reg_pagina->pag = pagina;
 reg_pagina->direccion_fisica = direccion_fisica;
 return reg_pagina;
}
static void pag_proc_destroy(pag_proceso * self)
{
 free(self);
}

/* ----------------------MEMORIA -----------------*/
t_list * crearListaFrames()
{
	 t_list * lista_frames = list_create();
	 return lista_frames;
}

t_list * crearListaHuecosFrames(int cantidad_marcos, int tamanio_marco, char* memoria_real)
{
	int x = 0;
	int desplazamiento = 0;
	t_list * lista_huecos_frames = list_create();
	while ( x != cantidad_marcos)
	{
	  desplazamiento = x * tamanio_marco;
	  int a = list_add(lista_huecos_frames, marco_hueco_create(memoria_real + desplazamiento, x));
	  x ++;
	}
	return lista_huecos_frames;
}

t_list * crearListaTlb()
{
 t_list * lista_tlb = list_create();
 return lista_tlb;
}

t_list * crearListaProceso()
{
	 t_list * lista_proceso = list_create();
	 return lista_proceso;
}


t_list * crearListaAdm()
{
	 t_list * lista_administracion = list_create();
	 return lista_administracion;
}
