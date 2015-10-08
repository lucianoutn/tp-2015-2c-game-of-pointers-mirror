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
#include <errno.h>

#define N 50

int descriptoresVec[N], maxDescriptor, j;
fd_set readset;

void reciboDelCpu(char *, t_list *, t_list *);

int main()
{
 traigoContexto();
 creoLogger(1);

 // RESERVO ESPACIO PARA LA MEMORIA REAL /
 int tamanio_memoria_real = miContexto.tamanioMarco * miContexto.cantidadMarcos;
 char * memoria_real = reservarMemoria(tamanio_memoria_real);
 t_list * TLB = NULL;
 // CREO UNA LISTA PARA REFERENCIAR A LAS LISTAS DE PROCESOS //
 t_list * tablaAdm = crearListaAdm();
 // CREO LISTAS PARA LOS FRAMES LLENOS Y VACIOS DE MEMORIA REAL
 listaFramesMemR = crearListaFrames();
 listaFramesHuecosMemR = crearListaHuecosFrames(miContexto.cantidadMarcos, miContexto.tamanioMarco, memoria_real);


 /* SI LA TLB NO ESTA HABILTIADA, VA A APUNTAR A NULL */
 if (!strcmp(miContexto.tlbHabilitada,"SI"))
 {
   TLB = crearListaTlb();
   printf ("La TLB esta habilitada \n");
   int tamanio_memoria_cache = miContexto.tamanioMarco * 4;
   //t_list * TLB = crearListaTlb();
  }

  reciboDelCpu(&memoria_real, TLB, tablaAdm);

  log_destroy(logger);
  return 0;
}

void reciboDelCpu(char * memoria_real, t_list * TLB, t_list * tablaAdm)
{
	int serverSocket;
	int listenningSocket, socketCliente, resultadoSelect;
	fd_set readset;
	t_header * package = malloc(sizeof(t_header));
	char * mensaje;
	int status = 1;

	//conexionAlCliente(&listenningSocket, &socketCliente, miContexto.puertoServidor);
	printf("Administrador de memoria conectado al CPU\n. Esperando mensajes:\n");
	printf("El socket de conexión con el CPU es %d\n", socketCliente);

	// ME CONECTO AL SWAP PARA ENVIARLE LO QUE VOY A RECIBIR DE LA CPU
	serverSocket = meConectoAlSwap();

	/*PRUEBAS

	while(status!=0)
	{
		//sem_wait(sem_1);
		// RECIBO EL PAQUETE(t_header) ENVIADO POR LA CPU
		status = recv(socketCliente, package, sizeof(t_header), 0);

		printf ("l tipo de ejecucion recibido es %d \n", package->type_ejecution);

		//
	  	  if(package.tamanio_msj!=0)
	  	  {
		  mensaje = malloc(package.tamanio_msj);
		  status = recv(socketCliente, mensaje, package.tamanio_msj,0);
	  	  }
		//

		// MANDO EL PAQUETE RECIBIDO A ANALIZAR SU TIPO DE INSTRUCCION PARA SABER QUE HACER
		ejecutoInstruccion(package, mensaje, memoria_real, TLB, tablaAdm, socketCliente, serverSocket);
	}
FIN PRUEBAS
*/

	t_header * package1 = malloc(sizeof(t_header));
	package1->type_ejecution=2;
	package1->PID=1;
	package1->pagina_proceso=3;

	ejecutoInstruccion(package1, mensaje, memoria_real, TLB, tablaAdm, socketCliente, serverSocket);

	package1->type_ejecution=0;
	package1->PID=1;
	package1->pagina_proceso=0;

	ejecutoInstruccion(package1, mensaje, memoria_real, TLB, tablaAdm, socketCliente, serverSocket);
  /* SELECT primera version no borrar
  do {
     FD_ZERO(&readset); 	//esto abre y limpia la estructura cada vez q se reinicia el select luego de un error
     FD_SET(socketCliente, &readset);
     resultadoSelect = select(socketCliente + 1, &readset, NULL, NULL, NULL); //el 1ºparametro es el socket +1, 2º el conjunto de lecutra, 3º el de escritura, 4º no se, 5º el time out
  } while (resultadoSelect == -1 && errno == EINTR); //captura el error

  if (resultadoSelect > 0) {	//>0 implica el nº de sockets disponibles para la lectura
     if (FD_ISSET(socketCliente, &readset)) {
        /* si estoy aca es que hay info para leer */
    /*
	resultadoSelect = recv(socketCliente, mensaje, package->tamanio_msj,0);
    if (resultadoSelect == 0)
    {
       // si estoy aca es xq se cerro la conexion desde el otro lado //
       close(socketCliente);
    }
    else if (resultadoSelect < 0)
    {
    	// error, lo muestro
    	printf("Error con el select(): %s\n ", strerror(errno));
    }
    */
    // fin primera version
  /* tercera version
  iDEM ARRIBA, SE COMENTA PARA PRUEBAS

  // inicializar el conjunto
  FD_ZERO(&readset); //esto abre y limpia la estructura cada vez q se reinicia el select luego de un error
  maxDescriptor = 0;
  for (j=0; j<N; j++) {
     FD_SET(descriptoresVec[j], &readset);
     //maxDescriptores = (maxDescriptores>descriptoresVec[j])?maxDescriptores:descriptoresVec[j];
     if (descriptoresVec[j] > maxDescriptor )
       	 maxDescriptor = descriptoresVec[j];
  }

  // se fija si hay algo para leer
  resultadoSelect = select(maxDescriptor + 1, &readset, NULL, NULL, NULL);  //el 1ºparametro es el socket +1, 2º el conjunto de lecutra, 3º el de escritura, 4º no se, 5º el time out
  if (resultadoSelect < 0) {
	  /* error, lo muestro
	 printf("Error con el select()");
  }
  else { 								//>0 implica el nº de sockets disponibles para la lectura
     for (j=0; j<N; j++) {
        if (FD_ISSET(descriptoresVec[j], &readset)) {
        	 /* si estoy aca es que hay info para leer

        }
     }
  }
  FIN COMENTARIO DE PRUEBAS
  fin tercera version */
  //FIN SELECT

  //status = recv(socketCliente, mensaje, package.tamanio_msj,0);
  /*SWAP
  int serverSocket;
  conexionAlServer(&serverSocket, miContexto.puertoCliente);
   //FIN SWAP
    */

/*
 t_header * package2 = package_create(1,4,0,2);

 ejecutoInstruccion(package2, mensaje, memoria_real, TLB, tablaAdm, socketCliente);
 sleep(miContexto.retardoMemoria);

  if(status!= 0)
  {
*/

		  //send(serverSocket, &package, sizeof(t_header), 0);
/*
		  if(package.tamanio_msj!=0)
		  {
			  //send(serverSocket, mensaje, strlen(mensaje), 0);
		  }
	  }
	*/
	  //sem_post(sem_2);

	//meConectoAlSwap(package,mensaje);

	//close(listenningSocket);
	//close(socketCliente);
	//close(serverSocket); //SWAP

}
