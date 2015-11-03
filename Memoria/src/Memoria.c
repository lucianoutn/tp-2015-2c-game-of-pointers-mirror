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
	t_header * package = malloc(sizeof(t_header));
	char * mensaje = malloc(miContexto.tamanioMarco);
	//int socketCPU;
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
    FD_ZERO(&master);    // borra los conjuntos maestro y temporal
    FD_ZERO(&read_fds);        // obtener socket a la escucha

    struct addrinfo hints; //estructura que almacena los datos de conexion
    struct addrinfo *serverInfo; //estructura que almacena los datos de conexion

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
    /*
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(miContexto.puertoServidor);
    memset(&(myaddr.sin_zero), '\0', 8);
    */
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
    	if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
    	{
    		perror("select");
    		exit(1);
    	}
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
    						printf("selectserver: socket %d hung up\n", i);
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
    								// MANDO EL PAQUETE RECIBIDO A ANALIZAR SU TIPO DE INSTRUCCION PARA SABER QUE HACER
    								ejecutoInstruccion(package, mensaje, memoria_real, TLB, tablaAdm, j, serverSocket);
    							}
 							}
    					}
    				}
    			} // Esto es ¡TAN FEO!
    		}
    	}
    }

}






