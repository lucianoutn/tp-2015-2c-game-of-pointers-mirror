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
#include <SharedLibs/libreriaServidor.h>
#include <SharedLibs/libreriaCliente.h>
#include <commons/config.h>
#include <commons/string.h>
#include "funcSwap.h"
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>           // para las constantes O_* de los semaforos ipc
#include <sys/stat.h>        // para las constantes de modo de los semaforos ipc

#define BACKLOG 10

void reciboDelAdminMem();

int main()
{
	//-------Contexto--------------------------//
	traigoContexto();
	creoLogger(1); //recive 0 para log solo x archivo| recive 1 para log x archivo y x pantalla
	log_info(logger, "Inicio Log SWAP", NULL);
	//------Fin Contexto----------------------//

	//sem_t * semPrueba = sem_open("semPrueba", O_CREAT, 0644, 3);
	//Creo particion y listas
	archivo = crearParticion();
	lista_paginas = crearListaPaginas();
	lista_huecos = crearListaHuecos(contexto->cant_paginas);

/*
	t_header * package1 = malloc(sizeof(t_header));
	package1->type_ejecution=2;
	package1->PID=1;
	package1->pagina_proceso=3;
	analizoPaquete(package1,0);


	package1->type_ejecution=1;
	package1->PID=1;
	package1->pagina_proceso=0;
	package1->tamanio_msj=strlen("Hola\0")+1;
	analizoPaquete(package1,0);

	package1->type_ejecution=0;
	package1->PID=1;
	package1->pagina_proceso=0;
	package1->tamanio_msj=strlen("Hola\0")+1;
	analizoPaquete(package1,0);

*/
	reciboDelAdminMem();

	//Cierro particion
	cerrarParticion();
	log_destroy(logger);
	free(contexto);
	return 1;
}

void reciboDelAdminMem()
{
	semConexion = sem_open("semConexion", O_CREAT, 0644, 1);
	sem_wait(semConexion); //hasta que no cree el server, que no opere la memoria
	int listenningSocket=crearServer(contexto->puerto);

	//Estructura que tendra los datos de la conexion del cliente MEMORIA
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int L = listen(listenningSocket, BACKLOG);
	if (L==-1)
		perror("LISTEN");

	sem_post(semConexion);
	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr,	&addrlen);
	printf("Conexion aceptada Socket= %d \n",socketCliente);

	t_header * package = malloc(sizeof(t_header));
	int status = 2; // Estructura que maneja el status de los receive.

	printf("Cliente conectado. Esperando mensajes:\n");

	while(status>0)
		//Una vez conectado el cliente..
	{
		status = recv(socketCliente, package, sizeof(t_header), 0);
		//Recibido el paquete lo proceso..
		if(status > 0){
			analizoPaquete(package,socketCliente);
			sleep(contexto->retardo_swap);
		}
	}


	//Cierro conexiones
	close(socketCliente);
	close(listenningSocket);
	status = 0;
}
