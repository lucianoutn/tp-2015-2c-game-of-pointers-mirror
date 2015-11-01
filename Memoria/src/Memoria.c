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
#include <wait.h>


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

	//RESERVO ESPACIO PARA LA MEMORIA REAL /
	char * memoria_real = reservarMemoria(miContexto.cantidadMarcos, miContexto.tamanioMarco);
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
		//t_list * TLB = crearListaTlb();
	}
 	//Me quedo atenta a las señales, y si las recibe ejecuta esa funcion
	// Se inicializa el mutex
	pthread_t senial[3];
	pthread_mutex_init (&mutexTLB, NULL);
	pthread_mutex_init (&mutexMem, NULL);
	void flush ()
	{
		int err= pthread_create(&(senial[0]), NULL, (void*)tlbFlush,TLB);
		if (err != 0)
			printf("no se pudo crear el hilo de TLBFlush :[%s]", strerror(err));
		pthread_join(senial[0], NULL);
	}
	void limpiar()
	{
		parametros * param=malloc(sizeof(parametros));

		param->memoria= memoria_real;
		param->tabla_adm = tablaAdm;
		param->tlb=TLB;

		printf("La direccion de la mem real es: %p \n",memoria_real);

		int err= pthread_create(&(senial[0]), NULL, (void*)limpiarMemoria,param);
		if (err != 0)
			printf("no se pudo crear el hilo de limpiarMemoria :[%s]", strerror(err));
		//pthread_join(senial[0], NULL);
	}
	void dump()
	{
		pid_t idProceso;
		idProceso =fork();
		switch(idProceso)
		{
		case -1:
			log_error(logger,"Error en la creacion del hijo");
			break;
		case 0:
			puts("Entro en case 0");
			dumpEnLog(memoria_real,tablaAdm);
			exit(0);
			break;

		default:
			sleep(3);
			puts("Entro al default");

		}
	}

	void mostrarTasas()
	{
		puts("Muestro tasas de cosas");

		signal(SIGALRM,mostrarTasas);

		alarm(60);

	}

	signal(SIGUSR1,flush);
	signal(SIGUSR2,limpiar);
	signal(SIGINT,dump);

	signal(SIGALRM,mostrarTasas);
	alarm(60);

	reciboDelCpu(memoria_real, TLB, tablaAdm);

	free(memoria_real);
	list_destroy_and_destroy_elements(tablaAdm,(void*)tabla_adm_destroy);
	//list_destroy_and_destroy_elements(TLB,(void*)reg_tlb_destroy);
	list_destroy_and_destroy_elements(listaFramesHuecosMemR,(void*)marco_hueco_destroy);
	list_destroy_and_destroy_elements(listaFramesMemR,(void*)marco_destroy);
	log_destroy(logger);

//	pthread_kill(senial[0],3);
	return 1;
}

void reciboDelCpu(char * memoria_real, t_list * TLB, t_list * tablaAdm)
{
	//_header * package = malloc(sizeof(t_header));
	char * mensaje = malloc(miContexto.tamanioMarco);
	int socketCPU;
	int resultadoSelect;
	fd_set readset;
	int status = 1;
/*
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
*/
	// ME CONECTO AL SWAP PARA ENVIARLE LO QUE VOY A RECIBIR DE LA CPU
	int serverSocket = crearCliente(IP,miContexto.puertoCliente);
/*
	while(status!=0)
	{
		// RECIBO EL PAQUETE(t_header) ENVIADO POR LA CPU
		status = recv(socketCPU, package, sizeof(t_header), 0);

		printf ("El tipo de ejecucion recibido es %d \n", package->type_ejecution);

		// MANDO EL PAQUETE RECIBIDO A ANALIZAR SU TIPO DE INSTRUCCION PARA SABER QUE HACER
		ejecutoInstruccion(package, mensaje, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);
	}
*/

	t_header * package = package_create(2,15,5,0);
	 char * mensaje_inicializacion = malloc(1);
	 ejecutoInstruccion(package, mensaje_inicializacion, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	 t_header * package_escritura = package_create(1,15,1,strlen("Hola"));
	 char * mensaje_escritura = malloc(5);
	 strcpy(mensaje_escritura,"Hola");
	 ejecutoInstruccion(package_escritura, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	 t_header * package_escritura1 = package_create(1,15,2,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Aldu");
	 ejecutoInstruccion(package_escritura1, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	 t_header * package_escritura2 = package_create(1,15,3,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Chau");
	 ejecutoInstruccion(package_escritura2, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	 t_header * package_escritura3 = package_create(1,15,1,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Tutu");
	 ejecutoInstruccion(package_escritura3, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	 t_header * package_escritura4 = package_create(1,15,4,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Gege");
	 ejecutoInstruccion(package_escritura4, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	 t_header * package_escritura5 = package_create(1,15,2,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Palo");
	 ejecutoInstruccion(package_escritura5, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	 t_header * package_escritura6 = package_create(1,15,3,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Pali");
	 ejecutoInstruccion(package_escritura6, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	 t_header * package_escritura7 = package_create(1,15,1,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Puli");
	 ejecutoInstruccion(package_escritura7, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	 t_header * package_finalizacion = package_create(3,15,0,0);
	 char * mensaje_finalizacion = malloc(1);
	 ejecutoInstruccion(package_finalizacion, mensaje_finalizacion, memoria_real, TLB, tablaAdm, socketCPU, serverSocket);

	 free(mensaje);
	 free(mensaje_inicializacion);
	 free(mensaje_escritura);
	 free(mensaje_finalizacion);
	 free(package);
	 free(package_escritura);
	 free(package_escritura1);
	 free(package_escritura2);
	 free(package_escritura3);
	 free(package_escritura4);
	 free(package_escritura5);
	 free(package_escritura6);
	 free(package_finalizacion);


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

}
