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
#include <pthread.h>

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
   int marco; // numero de marco ( si tiene ) en donde esta guardada la pagina
   int accessed;
   int dirty;
}process_pag;
// ------------------------------------//

//DECLARACIONES GLOBALES PARA MOSTRAR TASA DE ACIERTOS DE LA TLB.
int cantAccesosTlb;
int cantHitTlb;

// DECLARACIONES GLOBALES PARA MOSTRAR FALLOS DE PAGINA VS TOTAL DE PAGINAS ACCEDIDAS
int cantPagAccessed;
int cantFallosPag;

// ESTRUCTURA QUE CONTIENE EL CONTEXTO DE LA MEMORIA
contexto miContexto;

/* DECLARACIONES GLOBALES PARA CONOCER EL ESTADO DE LOS FRAMES DESDE CALQUIER LADO Y NO ANDAR PASANDO LA LISTA
 * DE FUNCION EN FUNCION
 */
t_list * listaFramesMemR; // LISTA DE FRAMES OCUPADOS
t_list * listaFramesHuecosMemR; // LISTA DE FRAMES VACIOS

// FUNCION QUE OBTIENE EL CONTEXTO DE LA MEMORIA Y LO GUARDA EN "miContexto" (variable global)
void traigoContexto();

/* RESERVO UNA CANTIDAD DE MEMORIA PASADA EN BYTES POR PARAMATRO
 * SE USA PARA RESERVAR EL TAMAÑO DE LA MEMORIA REAL
 */
char * reservarMemoria(int, int);

// SE SOBREENTIENDE LA FUNCION
void liberarMemoria(char *);

/* RECIBE LA INSTRUCCION RECIBIDA DEL CPU Y SE HACE UN CASE DEL TIPO DE EJECUCION PARA SABER
   LAS ESTRUCTURAS A CREAR, QUE MANDAR AL SWAP, QUE RECIBIR, ETC... */
void ejecutoInstruccion(t_header *, char*,char*, t_list*, t_list*, int, int, t_list *);

/* AGREGO UN FRAME A LA LISTA DE FRAMES, SACO UNO DE LA LISTA DE FRAMES HUECOS. ESCRIBO EL FRAME CON LA
 	PAGINA RECIBIDA DEL SWAP */
void lectura(t_header *, t_list*, char*, char*, t_list *, process_pag *);

// SOLO CONEXION //
int meConectoAlSwap();
// ENVIO DE HEADER RETORNA UN 1 EN CASO DE EXITO Y 0 SI HUBO ALGUN ERROR EN SWAP PARA RECIBIR
void envioAlSwap(t_header *, int, char *, int *);
// PREGUNTO SI LA TLB ESTA LLENA, DEVUELVE TRUE SI ESTA LLENA
bool tlbLlena(t_list *);
// FUNCION PARA FINALIZAR PROCESOS, ME MATA LA TABLA DE ACCESOS, ASIQUE MOSTRAR EL VERSUS ANTES DE MATARPROCESO
void matarProceso(t_header *, t_list*, t_list *, t_list*);

/* ME DEVUELVE NULL SI LA PAGINA ESTA EN SWAP Y SINO ME DEVUELVE LA DIRECCION DEL MARCO */
//direccionDePag();

int leerEnMemReal(t_list*,t_list *, t_header *, int,int, char *, t_list*);

/* VERIFICA SI LA TABLA ESTA CARGADA EN LA TABLA DE TABLAS DE PROCESOS Y LA DEVUELVE */
t_list * obtenerTablaProceso(t_list*, int);

/* DEVUELVE LA PAGINA DE LA TABLA DEL PROCESO */
process_pag * obtenerPaginaProceso(t_list *, int);

// Se fija si la encuentra cargada en tlb, en ese caso se la envia al cpu
int leerDesdeTlb(int , t_list * , int, int, t_list*);

/*
 * Unicamente crea las tablas para manejarlo
 * no le asigna marcos a las paginas
 */
void iniciarProceso(t_list*, t_header*, t_list*);

/* BUSCA SI LA PAGINA A ESCRIBIR YA ESTA CARGADA EN TLB, SI LO ESTA LA ESCRIBE Y DEVUELVE 1, SINO DEVUELVE 0 */
int escribirDesdeTlb (t_list *,int, char*, t_header *, t_list*, t_list*, int);

// ABSTRACCION DE TODO LO QUE HACE EL ESCRIBIR
void escribirEnMemReal(t_header*, t_list*, t_list*, t_list*, int, int, char*);

// ESCRIBO EL MARCO TENIENDO EN CUENTA QUE EL MENSAJE QUE ME LLEGA PUEDE SER MÁS LARGO Y QUE YA
//PODIA LLEGAR A ESTAR ESCRITO DE ANTES CON ALGO MÁS LARGO
void escribirMarco(char*, char*);

// ASIGNA UN MARCO A UNA PAGINA RECIEN TRAIDA DEL SWAP, ACTUALIZA TABLA PROCESO, TLB
void asignarMarcoPagSwap(t_header*,char*, t_list*, t_list*, t_list*, int);

int swapeando(t_list*, t_list* , t_list *, char *, int, t_header *, t_list*, int);

// LE MANDO LA TABLA DEL PROCESO Y ME DEVUELVE 1 SI YA TIENE TODOS SUS MARCOS DISPONIBLES OCUPADOS
int marcosProcesoLlenos(t_list *);

/*CUANDO LLEGA UN REGISTRO PARA LEER O ESCRIBIR
 * 1. SI LA TLB ESTA LLENA, SE ELIMINA EL QUE ESTA HACE MAS TIEMPO (EL ULTIMO) Y SE AGREGA EL REGISTRO RECIEN LLEGADO
 * 2. SI LA TLB TIENE ESPACIO, SE AGREGA
 */
void actualizarTlb(int, int, char*, t_list *, int);

void actualizoTablaProceso(t_list*, t_marco_hueco* ,t_header *);

/* ACTUALIZA LA TABLA PARA ALGORITMO LRU!!, AGREGANDO LA DIRECCION DE MEMORIA DEL MARCO A LA PAGINA
 * PERO A LA VEZ, LA ELIMINA DE LA TABLA Y LA PONE AL FINAL, PARA SEGUIR LA LOGICA
 * DE QUE SE VAYAN ELIMINANDO LAS PRIMERAS QUE SE CARGARON CUANDO ESTE LLENA (CANTIDAD DE MARCOS
 * PARA EL PROCESO)
 */
void actualizarTablaProcesoLru(t_list *, int ,char*,int);

/* ACTUALIZA LA TABLA PARA ALGORITMO FIFO!!, AGREGANDO LA DIRECCION DE MEMORIA DEL MARCO A LA PAGINA
 * EN CASO DE QUE NO LO TENGA Y MANDANDOLA AL FINAL DE LA LISTA, Y SI YA TENIA DIRECCION NO HACE NADA
 */
void actualizarTablaProcesoFifo(t_list *, int, char*, int);

// ATUALIZA LA TABLA PARA ALGORITMO CLOCK
void actualizarTablaProcesoClock(t_list *, int, char *, int);


// BUSCO Y RETORNO EL REGISTRO DE LA TLB BUSCADO EN EL CASO DE ENCONTRARLO, SINO RETORNO NULL.
// TAMBIEN CARGO EN EL TERCER PARAMETRO (INT*) LA POSICION DE ESA ENTRADA
t_tlb * buscarEntradaProcesoEnTlb (t_list*, t_header *, int*);

// VERIFICO SI EL PROCESO YA TIENE ALGUNA PAGINA CARGDA EN MEMORIA
int procesoTienePaginaCargada();

// REMUEVO MARCO POR NUMERO DE MARCO Y AGREGO A LA LISTA DE HUECOS
void removerMarcoPorMarco(int);

// CUANDO TENGO QUE ESCRIBIR, Y TENGO ESPACIO PARA GUARDAR LA PAGINA (NO TENGO QUE SWAPEAR), SOLO LE MANDO UN T_HEADER DE TIPO LECTURA AL SWAP
t_header * crearHeaderLectura(t_header*);

// CUANDO TENGO QUE ESCRIBIR, Y NO TENGO ESPACIO PARA GUARDAR LA PAGINA (TENGO QUE SWAPEAR),
// LE MANDO LA PAGINA A SWAPEAR PARA QUE LA ESCRIBA Y PODER ALMACENAR EN MEMORIA LA NUEVA A ESCRIBIR
t_header * crearHeaderEscritura(int, int, int);

// DEVUELVE LA PAGINA CORRESPONDIENTE PARA REMOVER DEPENDIENDO DEL ALGORITMO
process_pag * traerPaginaARemover(t_list *);

// OBTENGO LA PRIMER PAGINA QUE FUE CARGADA EN MEMORIA
// SI POR ALGUN MOTIVO NO LA ENCONTRO, RETORNO NULL (SIRVE SI ESTOY USANDO FIFO O LRU)
process_pag * paginaARemoverFifoLru(t_list*);

process_pag * paginaARemoverClock(t_list*);

// VERIFICA SI HAY ALGUNA PAGINA CON EL BIT ACCEDIDO Y EL DIRTY EN 0
process_pag * ambosBitsEnCero(t_list *);

// VERIFICA SI HAY ALGUNA PAGINA CON A = 0, D = 1, MIENTRAS NO LA ENCUENTRE,
// VA ACTUALIZANDO A = 0, SI RECORRIO TODO Y NO LA ENCONTRO, VUELVE A HACER AMBOSBITSENCERO
process_pag * bitDirtyEnUno(t_list*);

void upPaginasAccedidas(t_list*, int);
void upFallosPagina(t_list*, int);

void mostrarVersus(t_list*, int);

int removerEntradasTlb(t_list *, t_header*);

//------SEÑALES QUE TIENE QUE RECIBIR LA MEMORIA-------------//
void tlbFlush(t_list *);

void limpiarMemoria(char *, t_list*, t_list*);

void dumpEnLog();
//-----------------------------------------------------------//
void tasasDeTLB();


/* PASAR A MANEJOLISTAS */
process_pag * pag_proc_create(int, char*, int, int, int);

void pag_proc_destroy(process_pag *);
/* ---------------------*/

#endif /* SRC_FUNCMEMORY_H_ */

