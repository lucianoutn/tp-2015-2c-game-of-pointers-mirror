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

void reciboDelCpu(char *, t_list *, t_list *, t_list*);

int main() {
	traigoContexto();
	creoLogger(1); //recive 0 para log solo x archivo| recive 1 para log x archivo y x pantalla
	log_info(logger, "Inicio Log MEMORIA", NULL);

	char * memoria_real = reservarMemoria(miContexto.cantidadMarcos,miContexto.tamanioMarco); // Reservo espacio para memoriareal
	t_list * TLB = NULL;
	t_list * tablaAdm = crearListaAdm(); // CREO UNA LISTA PARA REFERENCIAR A LAS LISTAS DE PROCESOS
	listaFramesMemR = crearListaFrames(); // CREO LISTAS PARA LOS FRAMES LLENOS Y VACIOS DE MEMORIA REAL
	listaFramesHuecosMemR = crearListaHuecosFrames(miContexto.cantidadMarcos,miContexto.tamanioMarco, memoria_real);
	t_list * tablaAccesos = crearListaVersus(); // CREO LISTA PARA GUARDAR LAS PAGINAS ACCEDIDAS Y LOS FALLOS DE CADA PROCESO

	/* SI LA TLB NO ESTA HABILTIADA, VA A APUNTAR A NULL */
	if (!strcmp(miContexto.tlbHabilitada, "SI")) {
		TLB = crearListaTlb();
		printf("La TLB esta habilitada \n");
		//t_list * TLB = crearListaTlb();
		//Inicializo contadores
		cantAccesosTlb = 0;
		cantHitTlb = 0;
	}
	//Me quedo atenta a las señales, y si las recibe ejecuta esa funcion
	seniales(TLB, memoria_real, tablaAdm);

	reciboDelCpu(memoria_real, TLB, tablaAdm, tablaAccesos);

	free(memoria_real);
	list_destroy_and_destroy_elements(tablaAdm, (void*) tabla_adm_destroy);
	//list_destroy_and_destroy_elements(TLB,(void*)reg_tlb_destroy);
	list_destroy_and_destroy_elements(listaFramesHuecosMemR,(void*) marco_hueco_destroy);
	list_destroy_and_destroy_elements(listaFramesMemR, (void*) marco_destroy);
	log_destroy(logger);
	return 1;
}

void seniales(t_list* TLB, char* memoria_real, t_list * tablaAdm)
{
	void flush()
	{
		log_info(logger,"Se recibio SIGUSR1, si corresponde se vacia la TLB.");
		//sleep(3);
		tlbFlush(TLB);
		log_info(logger, "Tratamiento de la señal SIGUSR1 terminado.");
	}
	void limpiar()
	{
		log_info(logger, "Se recibio SIGUSR2, se limpia la memoria.");
		//sleep(3);
		limpiarMemoria(memoria_real, TLB, tablaAdm);
		log_info(logger, "Se finalizo el tratamiento de la señal SIGUSR2.");
	}
	void dump()
	{
		log_info(logger,"Se recibio SIGPOLL, se muestra el contenido de la memoria actualmente");
		dumpEnLog(memoria_real, tablaAdm);
		log_info(logger, "Tratamiento de la señal SIGPOLL terminado");
	}
	void mostrar()
	{
		log_info(logger,"Luego de un minuto, muestro tasa de aciertos de la TLB");
		tasasDeTLB();
		alarm(60);
	}

	signal(SIGUSR1, flush);
	signal(SIGUSR2, limpiar);
	signal(SIGPOLL, dump);
	signal(SIGALRM,mostrar);
	alarm(60);
}

void reciboDelCpu(char * memoria_real, t_list * TLB, t_list * tablaAdm,t_list* tablaAccesos)
{
	t_header * package = malloc(sizeof(t_header));
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
	int serverSocket = crearCliente(IP, miContexto.puertoCliente);
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
	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	// struct sockaddr_in myaddr;     // dirección del servidor
	struct sockaddr_in remoteaddr; // dirección del cliente
	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	int newfd;        // descriptor de socket de nueva conexión aceptada
	int nbytes;
	int yes = 1;        // para setsockopt() SO_REUSEADDR, más abajo
	int addrlen;
	int i, j;

	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);        // obtener socket a la escucha

	struct addrinfo hints; //estructura que almacena los datos de conexion
	struct addrinfo *serverInfo; //estructura que almacena los datos de conexion
	int err=0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, miContexto.puertoServidor, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion
	if ((listener = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) == -1) {
		perror("socket");
		exit(1);
	}
	// obviar el mensaje "address already in use" (la dirección ya se está usando)
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
		perror("setsockopt");
		exit(1);
	}
	// enlazar
	if (bind(listener, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
		perror("bind");
		exit(1);
	}
	// escuchar
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(1);
	}
	// añadir listener al conjunto maestro
	FD_SET(listener, &master); // seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste
	// bucle principal
	for (;;)
	{
		read_fds = master; // cópialo

		select_restart:
		if ((err = select(fdmax+1, &read_fds, NULL, NULL, NULL)) == -1)
		{
			if (errno == EINTR)
			{  	// Alguna señal nos ha interrumpido, así que regresemos a select()
				puts("Me interrumpio una señal, pero no me cabe una y vuelvo al select \n");
				goto select_restart;
			}
			// Los errores reales de select() se manejan aquí:
			perror("select");
		}
		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &read_fds))
			{ // ¡¡tenemos datos!!
				if (i == listener)
				{
					// gestionar nuevas conexiones
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener,(struct sockaddr *) &remoteaddr, &addrlen)) == -1)
					{
						perror("accept");
					}
					else
					{
						FD_SET(newfd, &master); // añadir al conjunto maestro
						if (newfd > fdmax)
						{    // actualizar el máximo
							fdmax = newfd;
						}
						printf("Nueva conexion\n ");
					}
				} else
				{
					// gestionar datos de un cliente
					if ((nbytes = recv(i, package, sizeof(t_header), 0)) <= 0)
					{
						// error o conexión cerrada por el cliente
						if (nbytes == 0)
						{
							// conexión cerrada
							printf("Conexion del socket %d cerrada \n", i);
						} else
						{
							perror("recv");
						}
						close(i);
						// ¡Hasta luego!
						FD_CLR(i, &master);
						// eliminar del conjunto maestro
					} else
					{
						// tenemos datos de algún cliente
						for (j = 0; j <= fdmax; j++)
						{
							if (FD_ISSET(j, &master))
							{
								if (j != listener)
								{
									printf("El tipo de ejecucion recibido es %d \n",package->type_ejecution);
									char * mensaje = malloc(package->tamanio_msj);
									if (package->type_ejecution == 1) //Escritura
									{
										recv(i, mensaje, package->tamanio_msj,0);
										mensaje[package->tamanio_msj] = '\0';
										printf("Mensaje recibido: <%s>  Tamaño: %d.\n",	mensaje, package->tamanio_msj);
									}
									else
									{
										mensaje = NULL;
									}
									// MANDO EL PAQUETE RECIBIDO A ANALIZAR SU TIPO DE INSTRUCCION PARA SABER QUE HACER
									ejecutoInstruccion(package, mensaje,memoria_real, TLB, tablaAdm, j,serverSocket, tablaAccesos);
									free(mensaje);
								}
							}
						}
					}
				} // Esto es ¡TAN FEO!
			}
		}
	}

}
