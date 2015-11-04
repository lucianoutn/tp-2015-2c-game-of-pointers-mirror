/*
 * funcSwap.h
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */

#ifndef FUNCSWAP_H_
#define FUNCSWAP_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <SharedLibs/manejoListas.h>
#include <unistd.h>
#include <SharedLibs/comun.h>

//Estructura para traer los datos del archivo de configuracion.
typedef struct{
	char * puerto;
	char * nombre;
	int cant_paginas;
	int tam_pagina;
	int retardo_swap;
	int retardo_compac;
}t_contexto;


//Declaro variables globales
t_contexto * contexto;
FILE* archivo;
t_list * lista_paginas;
t_list * lista_huecos;
t_header * global;

sem_t * semConexion;

/*
 * Crea el archivo swap y devuelve un puntero al archivo abierto
 */
FILE* crearParticion();

/*
 * Cierra el archivo swap
 */
void cerrarParticion();

/*
 * Lee el archivo de configuracion y lo carga en la variable contexto
 */
void traigoContexto();

char * numero_de_pagina(char *);

/*
 * Abre el t_header, y segun el tipo de ejecucion
 * loggea por pantalla que instruccion recibio,llama a
 * la funcion correspondiente.
 * Segun lo que devolvio la funcion, le manda un t_devuelvo a Memoria
 */
void analizoPaquete(t_header *, int );

/*
 * Recibe el t_header con la info necesaria, y se la pasa por parametro un
 * char * donde se devuelve el contenido.
 * Busca en la lista de paginas la que coincida con el numero de pid para saber en que
 * byte comienza esa pagina en el archivo y posicionarse para leer.
 * En caso de no encontrar en la lista de paginas, loggea error, y devuelve nulo.
 */
void leerSwap(t_header*,char *);
/*
 * Recibe el t_header con la info necesaria, recibe por socket el mensaje que va a escribir.
 * Busca en la lista de paginas la que coincida con el numero de pid para saber en que
 * byte comienza esa pagina en el archivo y posicionarse para escribir.
 * Rellena el resto de la pagina con \0.
 * En caso de no encontrar en la lista de paginas, loggea error, y devuelve nulo.
 */
int escribirSwap(t_header*, int);

/*
 * Recibe un t_header. Busca un hueco que coincida con la cantidad que de paginas del t_header.
 * y agrega un nodo a la lista de paginas. Luego actualiza la lista de huecos.
 * Retorna 1 si esta todo bien. 0 si hubo error.
 */
int inicializarProc(t_header*);

/*
 * Recibe un t_header. Busca en la lista de paginas un nodo que coincida con ese PID.
 * Lo agrega a la lista de huecos y lo remueve de la lista de paginas.
 */
int finalizarProc(t_header*);

/*
 * Toma un elemento de la lista, lo pone al principio de esta. Guarda el contenido de todas
 * las paginas juntas. Calcula el nuevo inico en cuando al inicion del nodo que se guardo antes
 * y la cantidad de paginas que ocupa, y escribe el contenido en la nueva poiscion.
 * Luego, actualiza la lista de huecos, para dejar uno solo, al final de la particion.
 */
void compactarSwap();

/*
 * Recibe la lista de paginas que cada uno tiene su nodo t_pag con la cantidad de veces
 * que fue leida y escrita esa pag.
 * Realiza la sumatoria y la loggea.
 */
void leidasYEscritas(t_list *);

void rellenarParticion(int, int);

/*
 * Recibe la cantidad de paginas que se esta buscando ubicar. Recorre la lista de huecos
 * hasta encontrar un hueco cuya cantidad de paginas que ocupa sea mayor o igual a la cantidad
 * que se necesita.
 * Si no encuentra un hueco, chequea si es por falta de espacio o fragmentación externa
 * y en caso de esta ultima, hace la compactacion, y devuelve el primer hueco, ya que es el unico.
 */
t_hueco* buscarHueco(int);

/*
 * Funcion booleana para utilizar en el list_find
 * que recibe el numero de pid, y chequea si es iguqal al pid
 * que se guardo en la variable global.
 */
bool numeroDePid(int *);

//Manejo de listas
t_list * crearListaPaginas();

t_list * crearListaHuecos(int);
#endif /* FUNCSWAP_H_ */
