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
#include <SharedLibs/libreriaCliente.h>
#include <SharedLibs/libreriaServidor.h>
#include "funcMemory.h"
#include <errno.h>
#include <fcntl.h>           // para las constantes O_* de los semaforos ipc
#include <sys/stat.h>        // para las constantes de modo de los semaforos ipc
#include <sys/types.h> 		//Para señales
#include <signal.h>			//Para señales

#define IP "127.0.0.1"
#define BACKLOG 10
#define N 50

int descriptoresVec[N], maxDescriptor, j;
fd_set readset;

void reciboDelCpu(char *, t_list *, t_list *);

int main()
{
	traigoContexto();
	creoLogger(1);  //recive 0 para log solo x archivo| recive 1 para log x archivo y x pantalla
	log_info(logger, "Inicio Log MEMORIA", NULL);

	//sem_t * semPrueba= sem_open("semPrueba", 0);

	//RESERVO ESPACIO PARA LA MEMORIA REAL /
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

	list_add(TLB, reg_tlb_create(0, 1, 0x28823));
	list_add(TLB, reg_tlb_create(1,2,0x383883));
	list_add(TLB, reg_tlb_create(2,0,0x923823));
	int a= list_size(TLB);
	printf("La TLB tiene %d elementos",a);
 	//Me quedo atenta a las señales, y si las recibe ejecuta esa funcion
	signal(SIGUSR1,tlbFlush);
	signal(SIGUSR2,limpiarMemoria);
	signal(SIGPOLL,dumpEnLog);

	reciboDelCpu(memoria_real, TLB, tablaAdm);

	log_destroy(logger);
	return 0;
}

void reciboDelCpu(char * memoria_real, t_list * TLB, t_list * tablaAdm)
{
	int socketCPU;
	int resultadoSelect;
	fd_set readset;
	t_header * package = malloc(sizeof(t_header));
	char * mensaje=malloc(11);
	int status = 1;
	sem_t *semConexion= sem_open("semConexion", 0);

	//CONEXION AL CPU
	int listenningSocket=crearServer(miContexto.puertoServidor);

	//Estructura que tendra los datos de la conexion del cliente MEMORIA
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int L = listen(listenningSocket, BACKLOG);
	if (L==-1)
	 perror("LISTEN");

	socketCPU = accept(listenningSocket, (struct sockaddr *) &addr,	&addrlen);
	printf("Administrador de memoria conectado al CPU\n. Esperando mensajes:\n");
	printf("Conexion aceptada Socket= %d \n",socketCPU);


	sem_wait(semConexion);
	// ME CONECTO AL SWAP PARA ENVIARLE LO QUE VOY A RECIBIR DE LA CPU
	int serverSocket = crearCliente(IP,miContexto.puertoCliente);
	sem_post(semConexion);

	while(status!=0)
	{
		// RECIBO EL PAQUETE(t_header) ENVIADO POR LA CPU
		status = recv(socketCPU, package, sizeof(t_header), 0);

		printf ("El tipo de ejecucion recibido es %d \n", package->type_ejecution);

		/*
	  	  if(package->tamanio_msj!=0)
	  	  {
		  mensaje = malloc(package->tamanio_msj);
		  status = recv(socketCPU, mensaje, package->tamanio_msj,0);
	  	  }
		 */

		// MANDO EL PAQUETE RECIBIDO A ANALIZAR SU TIPO DE INSTRUCCION PARA SABER QUE HACER
		ejecutoInstruccion(package, mensaje, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);
	}
/*
	t_header * package1 = malloc(sizeof(t_header));
	package1->type_ejecution=2;
	package1->PID=1;
	package1->pagina_proceso=3;

	ejecutoInstruccion(package1, mensaje, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	package1->type_ejecution=0;
	package1->PID=1;
	package1->pagina_proceso=0;

	ejecutoInstruccion(package1, mensaje, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	package1->type_ejecution=0;
	package1->PID=1;
	package1->pagina_proceso=1;

	ejecutoInstruccion(package1, mensaje, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);
*/
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


/*
	close(listenningSocket);
	close(socketCPU);
	close(serverSocket); //SWAP
*/
}
