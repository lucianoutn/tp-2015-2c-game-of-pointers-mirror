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

void reciboDelCpu(char *, t_list *, t_list *);


int main()
{
 traigoContexto();

 t_list * TLB = NULL;
 // CREO UNA LISTA PARA REFERENCIAR A LAS LISTAS DE PROCESOS //
 t_list * listaTablasProcesos = crearListaAdm();
 // CREO LISTAS PARA LOS FRAMES LLENOS Y VACIOS DE MEMORIA REAL
 t_list * listaFramesMemR = crearListaFrames();
 t_list * listaFramesHuecosMemR = crearListaHuecosFrames();
 // RESERVO ESPACIO PARA LA MEMORIA REAL /
 int tamanio_memoria_real = miContexto.tamanioMarco * miContexto.cantidadMarcos;
 char * memoria_real = reservarMemoria(tamanio_memoria_real);

 /* SI LA TLB NO ESTA HABILTIADA, VA A APUNTAR A NULL */
 if (!strcmp(miContexto.tlbHabilitada,"SI"))
 {
   TLB = crearListaTlb();
   printf ("La TLB esta habilitada \n");
   int tamanio_memoria_cache = miContexto.tamanioMarco * 4;
   t_list * TLB = crearListaTlb();
  }

  reciboDelCpu(&memoria_real, TLB, listaTablasProcesos);
  return 0;
}

void reciboDelCpu(char * memoria_real, t_list * TLB, t_list * tablaAdm)

{
 int listenningSocket;
 int socketCliente;
 t_header package;

  conexionAlCliente(&listenningSocket, &socketCliente, miContexto.puertoServidor);
  printf("Administrador de memoria conectado al CPU\n. Esperando mensajes:\n");
  printf("El socket de conexión con el CPU es %d\n", socketCliente);

  int status = 1;
  char * mensaje;
  status = recv(socketCliente, mensaje, package.tamanio_msj,0);
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
