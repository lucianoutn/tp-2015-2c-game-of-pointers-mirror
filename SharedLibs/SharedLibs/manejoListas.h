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
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/collections/list.h>

// ------MANEJO EN SWAP----- //
typedef struct {
 int pid;
 int inicio;
 int paginas;
} t_pag;

typedef struct {
 int inicio;
 int paginas;
} t_hueco;
// ---------------------------//

// MANEJO DESDE CPU -> MEMORY -> SWAP /
typedef struct
{
 int type_ejecution;
 int PID;
 int pagina_proceso;
 int tamanio_msj;
} t_header;
// -------------------------------//

// MANEJO DE FRAMES EN MEMORIA //
typedef struct {
 char * direccion_inicio;
 int numero_marco;
} t_marco;

typedef struct {
 char* direccion_inicio;
 int numero_marco;
} t_marco_hueco;
// -----------------------------//

// ENTRADAS A LA TLB //
typedef struct
{
 int pid;
 int pagina;
 char * direccion_fisica;
} t_tlb;
// -------------------//

// STRUCT TABLA DE TABLAS DE PROCESOS //
typedef struct
{
 int pid;
 t_list * direc_tabla_proc;
} t_tabla_adm;
// ----------------------------//

// STRUCT TABLA PARA CADA PROCESO QUE LLEGA //
typedef struct
{
   int pag; // Contiene el numero de pagina del proceso
   char * direccion_fisica; //Contiene la direccion de memoria de la pagina que se esta referenciando
} pag_proceso;
// ------------------------------------//



// ---------- SWAP ----------------//
t_pag * pag_create(int,int, int);

void pag_destroy(t_pag *);

t_hueco * hueco_create(int, int);

void hueco_destroy(t_hueco *);
// .................................//

// ------MANEJO DESDE CPU -> MEMORY -> SWAP----//
t_header * package_create (int, int, int, int);

static void package_destroy(t_header *);
// ------------------------------------------- //

// -------- MEMORIA --------------------------//
t_marco * marco_create (char *, int);

void marco_destroy(t_marco *);

t_marco_hueco * marco_hueco_create (char * direc_mem, int);

void marco_hueco_destroy(t_marco_hueco * self);

t_tlb * reg_tlb_create (int pid, int marco, char * direc_mem);

void reg_tlb_destroy(t_tlb * self);

t_tabla_adm * tabla_adm_create(int, t_list*);

static void tabla_adm_destroy(t_tabla_adm *);

/* VER SI SE PUEDE PASAR ACA
pag_proceso * pag_proc_create(int, char*);
static void pag_proc_destroy(pag_proceso *);
/* ----------------------------------*/


/* ----------- MEMORIA ----------------*/
t_list * crearListaFrames();

t_list * crearListaHuecosFrames(int, int, char*);

t_list * crearListaTlb();

t_list * crearListaProceso();

t_list * crearListaAdm();

// -------------------------------------- //


#endif /* SHAREDLIBS_MANEJOLISTAS_H_ */
