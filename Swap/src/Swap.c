/*
 ============================================================================
 Name        : Swap.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Swap
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <SharedLibs/sockets.h>
#include <commons/config.h>
#include <commons/string.h>
#include "funcSwap.h"
#include <semaphore.h>


void reciboDelAdminMem();

int main()
{
	/*
	int a;
	a=sem_init(sem_2,1,1);
	a=sem_init(sem_1,1,0);
	*/
	//-------Contexto--------------------------//
	traigoContexto();
	creoLogger();
	//------Fin Contexto----------------------//

	//Creo particion y listas
	archivo = crearParticion();
	lista_paginas = crearListaPaginas();
	lista_huecos = crearListaHuecos(contexto->cant_paginas);

	reciboDelAdminMem();

	//Cierro particion
	cerrarParticion();
	log_destroy(logger);
}

void reciboDelAdminMem()
{
  int listenningSocket,socketCliente;
  t_header package;
  int status = 2; // Estructura que maneja el status de los receive.

  conexionAlCliente(&listenningSocket, &socketCliente,contexto->puerto);
  printf("Cliente conectado. Esperando mensajes:\n");

  while(status!=0)
  //Una vez conectado el cliente..
  {
	  //sem_wait(sem_2);
	  status = recv(socketCliente, &package, sizeof(t_header), 0);
	  //Recibido el paquete lo proceso..
	  if(status!= 0)
	  analizoPaquete(package,socketCliente);
	  sleep(contexto->retardo_swap);
	  //sem_post(sem_1);
  }


  //Cierro conexiones
  close(socketCliente);
  close(listenningSocket);
  status = 0;
}
