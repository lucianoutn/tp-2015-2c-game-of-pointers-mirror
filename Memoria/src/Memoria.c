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
	// CREO LISTA PARA GUARDAR LAS PAGINAS ACCEDIDAS Y LOS FALLOS DE CADA PROCESO
	t_list * tablaAccesos = crearListaVersus();

	/* SI LA TLB NO ESTA HABILTIADA, VA A APUNTAR A NULL */
	if (!strcmp(miContexto.tlbHabilitada,"SI"))
	{
		TLB = crearListaTlb();
		printf ("La TLB esta habilitada \n");
		//t_list * TLB = crearListaTlb();
		//Inicializo contadores
		cantAccesosTlb=0;
		cantHitTlb=0;
	}
 	//Me quedo atenta a las señales, y si las recibe ejecuta esa funcion
	void flush ()
		{
			log_info(logger, "Se recibio SIGUSR1, si corresponde se vacia la TLB \n");
			sleep(3);
			tlbFlush(TLB);
			log_info(logger, "Tratamiento de la señal SIGUSR1 terminado. \n");
		}
	void limpiar()
	{
		log_info(logger, "Se recibio SIGUSR2, se limpia la memoria \n");
		sleep(3);
		limpiarMemoria(memoria_real, TLB,tablaAdm);
		log_info(logger, "Se finalizo el tratamiento de la señal SIGUSR2 \n");
	}
	void dump()
	{
		log_info(logger, "Se rcibio SIGPOLL, se muestra el contenido de la memoria actualmente \n");
		sleep(3);
		dumpEnLog(memoria_real,tablaAdm);
		log_info(logger, "Tratamiento de la señal SIGPOLL terminado \n");
	}

	signal(SIGINT,flush);
	signal(SIGUSR2,limpiar);
	signal(SIGPOLL,dump);

	reciboDelCpu(memoria_real, TLB, tablaAdm, tablaAccesos);

	free(memoria_real);
	list_destroy_and_destroy_elements(tablaAdm,(void*)tabla_adm_destroy);
	//list_destroy_and_destroy_elements(TLB,(void*)reg_tlb_destroy);
	list_destroy_and_destroy_elements(listaFramesHuecosMemR,(void*)marco_hueco_destroy);
	list_destroy_and_destroy_elements(listaFramesMemR,(void*)marco_destroy);
	log_destroy(logger);
	return 1;
}

void reciboDelCpu(char * memoria_real, t_list * TLB, t_list * tablaAdm, t_list* tablaAccesos)
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
/*
	char * mensaje = malloc(1);

	t_header * package = package_create(2,15,5,0);
	 char * mensaje_inicializacion = malloc(1);
	 ejecutoInstruccion(package, mensaje_inicializacion, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

	 t_header * package_escritura = package_create(1,15,1,strlen("Hola"));
	 char * mensaje_escritura = malloc(5);
	 strcpy(mensaje_escritura,"Hola");
	 ejecutoInstruccion(package_escritura, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

	 t_header * package_lectura = package_create(0,15,2,0);
	 ejecutoInstruccion(package_lectura, NULL, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);


	 t_header * package_escritura1 = package_create(1,15,2,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Aldu");
	 ejecutoInstruccion(package_escritura1, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

	 t_header * package_escritura2 = package_create(1,15,3,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Chau");
	 ejecutoInstruccion(package_escritura2, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

	 t_header * package_lectura2 = package_create(0,15,3,0);
 	 ejecutoInstruccion(package_lectura2, NULL, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

	 t_header * package_escritura3 = package_create(1,15,1,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Tutu");
	 ejecutoInstruccion(package_escritura3, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

	 t_header * package_escritura4 = package_create(1,15,4,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Gege");
	 ejecutoInstruccion(package_escritura4, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

	 t_header * package_lectura3 = package_create(0,15,1,0);
 	 ejecutoInstruccion(package_lectura3, NULL, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

 	 t_header * package_lectura4 = package_create(0,15,4,0);
	 ejecutoInstruccion(package_lectura4, NULL, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

	 t_header * package_escritura5 = package_create(1,15,2,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Palo");
	 ejecutoInstruccion(package_escritura5, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

	 t_header * package_escritura6 = package_create(1,15,3,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Pali");
	 ejecutoInstruccion(package_escritura6, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

	 t_header * package_escritura7 = package_create(1,15,1,strlen("Aldu"));
	 strcpy(mensaje_escritura,"Puli");
	 ejecutoInstruccion(package_escritura7, mensaje_escritura, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

	 t_header * package_finalizacion = package_create(3,15,0,0);
	 char * mensaje_finalizacion = malloc(1);
	 ejecutoInstruccion(package_finalizacion, mensaje_finalizacion, memoria_real, TLB, tablaAdm, socketCPU, serverSocket, tablaAccesos);

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

*/
    fd_set master;   // conjunto maestro de descriptores de fichero
    fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
   // struct sockaddr_in myaddr;     // dirección del servidor
    struct sockaddr_in remoteaddr; // dirección del cliente
    int fdmax;        // número máximo de descriptores de fichero
    int listener;     // descriptor de socket a la escucha
    int newfd;        // descriptor de socket de nueva conexión aceptada
    int nbytes;
    int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
    int addrlen;
    int i, j;
    int r_select;

    int cronometro=0;
    FD_ZERO(&master);    // borra los conjuntos maestro y temporal
    FD_ZERO(&read_fds);        // obtener socket a la escucha

    struct addrinfo hints; //estructura que almacena los datos de conexion
    struct addrinfo *serverInfo; //estructura que almacena los datos de conexion
    struct timeval tv;
    tv.tv_sec=1;
    tv.tv_usec=0;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
    hints.ai_flags = AI_PASSIVE;// Asigna el address del localhost: 127.0.0.1
    hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

    getaddrinfo(NULL, miContexto.puertoServidor, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion
    if ((listener = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) == -1)
    {
    	perror("socket");
    	exit(1);
    }
    // obviar el mensaje "address already in use" (la dirección ya se está usando)
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1)
    {
    	perror("setsockopt");
    	exit(1);
    }
    // enlazar
    if (bind(listener, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1)
    {
    	perror("bind");
        exit(1);
    }
    // escuchar
    if (listen(listener, 10) == -1)
    {
    	perror("listen");
    	exit(1);
    }
    // añadir listener al conjunto maestro
    FD_SET(listener, &master);        // seguir la pista del descriptor de fichero mayor
    fdmax = listener; // por ahora es éste
    // bucle principal
    for(;;)
    {
    	read_fds = master; // cópialo
    	r_select=select(fdmax+1, &read_fds, NULL, NULL,	NULL); //HICE TRAMPA!!!
    	if(r_select == EINTR)
    	{
    		puts("Capture una señal \nnn");
    	}
    	else if (r_select== -1)
    	{
    		perror("select");
    		exit(1);
    	}
    	else if(r_select)
    	{
    		puts("Hay dato \n");
    	}
    	/*
    	else
    	{
    		cronometro=cronometro+1;
    		if(cronometro==60)
    		{
    			cronometro=0;
    			tasasDeTLB();
    		}
    	}
    	*/
    	// explorar conexiones existentes en busca de datos que leer
    	for(i = 0; i <= fdmax; i++)
    	{
    		if (FD_ISSET(i, &read_fds))
    		{ // ¡¡tenemos datos!!
    			if (i == listener)
    			{
    				// gestionar nuevas conexiones
    				addrlen = sizeof(remoteaddr);
    				if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen)) == -1)
    				{
    					perror("accept");
    				} else
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
    					for(j = 0; j <= fdmax; j++)
    					{
    						if (FD_ISSET(j, &master))
    						{
    							if(j!=listener)
    							{
    								printf ("El tipo de ejecucion recibido es %d \n", package->type_ejecution);

    								char * mensaje = malloc(package->tamanio_msj);

    								if(package->type_ejecution==1) //Escritura
    								{
    									recv(i,mensaje, package->tamanio_msj, 0);
    								}
    								else
    								{
    									strcpy(mensaje,"");
    								}
    								// MANDO EL PAQUETE RECIBIDO A ANALIZAR SU TIPO DE INSTRUCCION PARA SABER QUE HACER
    								ejecutoInstruccion(package, mensaje, memoria_real, TLB, tablaAdm, j, serverSocket, tablaAccesos);
    							}
 							}
    					}
    				}
    			} // Esto es ¡TAN FEO!
    		}
    	}
    }

}
