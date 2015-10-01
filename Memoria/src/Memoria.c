/*
 ============================================================================
 Name        : Memoria.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Memoria
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/config.h>
#include <SharedLibs/sockets.h>
#include "funcMemory.h"

void reciboDelCpu(char *, char *, t_list *, t_list *);


int main()
{
 traigoContexto();

 char * memoria_cache = NULL;
 t_list * TLB = NULL;
 // CREO UNA LISTA PARA LA TABLA DE LA MEMORIA REAL /
 t_list * listaTablaMemReal = crearListaTMemReal();
 // CREO UNA LISTA PARA LA MEMORIA REAL /
 t_list * listaMemR = crearListaMemReal();
 // RESERVO ESPACIO PARA LA MEMORIA REAL /
 int tamanio_memoria_real = miContexto.tamanioMarco * miContexto.cantidadMarcos;
 char * memoria_real = reservarMemoria(tamanio_memoria_real);

 /* SI LA TLB NO ESTA HABILTIADA, VA A APUNTAR A NULL AL IGUAL QUE LA MEMORIA CACHE
                  Y NO SE VA A RESERVAR ESPACIO PARA LA MISMA*/
 if (!strcmp(miContexto.tlbHabilitada,"SI"))
 {
   TLB = crearListaTlb();
   printf ("La TLB esta habilitada => Reservo espacio para la memoria cache \n");
   int tamanio_memoria_cache = miContexto.tamanioMarco * 4;
   memoria_cache = reservarMemoria (tamanio_memoria_cache);
   t_list * TLB = crearListaTlb();
  }

  reciboDelCpu(&memoria_real, &memoria_cache, TLB, listaTablaMemReal);
  return 0;
}

void reciboDelCpu(char * memoria_real, char * memoria_cache, t_list * TLB, t_list * tablaMemReal)

{
 int listenningSocket;
 int socketCliente;
 t_header package;

  conexionAlCliente(&listenningSocket, &socketCliente, miContexto.puertoServidor);
  printf("Administrador de memoria conectado al CPU\n. Esperando mensajes:\n");

  int status = 1;
  char * mensaje;
  //SWAP
  int serverSocket;
  conexionAlServer(&serverSocket, miContexto.puertoCliente);
   //FIN SWAP
  while(status!=0)
  {
	  //sem_wait(sem_1);
	  status = recv(socketCliente, &package, sizeof(t_header), 0);
	  if(package.tamanio_msj!=0)
	  {
		  mensaje = malloc(package.tamanio_msj);
		  status = recv(socketCliente, mensaje, package.tamanio_msj,0);
	  }

	  if(status!= 0)
	  {
		  send(serverSocket, &package, sizeof(t_header), 0);
		  if(package.tamanio_msj!=0)
		  	  {
			  send(serverSocket, mensaje, strlen(mensaje), 0);
		  	  }
	  }
	  //sem_post(sem_2);
  }

  //meConectoAlSwap(package,mensaje);

 close(listenningSocket);
 close(socketCliente);
 close(serverSocket); //SWAP

 }
