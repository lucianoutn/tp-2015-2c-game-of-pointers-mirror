/*
 * libreriaCliente.c
 *
 *  Created on: 11/9/2015
 *      Author: utnso
 */


#include "libreriaCliente.h";

//Conexion al servidor Planificador
int crearClientePlani(const char *IP, const char *PUERTOPLANIFICADOR){
///////////////////////////////////////////////////////////////

	//configuraciones varias
	memset(&hints_planificador, 0, sizeof(hints_planificador));
	hints_planificador.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints_planificador.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTOPLANIFICADOR, &hints_planificador, &serverInfo_planificador);	// Carga en serverInfo los datos de la conexion

	//se crea un nuevo socket que se utilizara para la conexion con el Planificador
	//descriptor del socket servidor
	int socketPlanificador = socket(serverInfo_planificador->ai_family, serverInfo_planificador->ai_socktype, serverInfo_planificador->ai_protocol);
	//se comprueba que el socket se creo correctamente
	if(socketPlanificador==-1)
		perror ("SOCKET PLANIFICADOR!");



/////////////////////////////////////////////////////////

	int C = connect(socketPlanificador, serverInfo_planificador->ai_addr, serverInfo_planificador->ai_addrlen);
	if (C==-1){
		perror ("CONNECT");
		return -1;
		}
	else
		printf ("Conexion con el Planificador lograda\n");


	freeaddrinfo(serverInfo_planificador);

	return socketPlanificador;
}


//Conexion al servidor Memoria
int crearClienteMem(const char *IP_MEMORIA, const char *PUERTOMEMORIA){

	////////////////////////////////////////////////////////

	//configuraciones varias
	memset(&hints_memoria, 0, sizeof(hints_memoria));
	hints_memoria.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints_memoria.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP_MEMORIA ,PUERTOMEMORIA,&hints_memoria,&serverInfo_memoria);

	//se crea un nuevo socket que se utilizara para la conexion con la memoria
	//descriptor del socket servidor
	int socketMemoria = socket(serverInfo_memoria->ai_family, serverInfo_memoria->ai_socktype, serverInfo_memoria->ai_protocol);
	//se comprueba que el socket se creo correctamente
	if(socketMemoria==-1)
		perror("SOCKET MEMORIA!");


	/////////////////////////////////////////////////

	int C2 = connect(socketMemoria, serverInfo_memoria->ai_addr, serverInfo_memoria->ai_addrlen);
	if (C2==-1){
		perror ("CONNECT");
		return -1;
		}
	else
		printf ("Conexion con la Memoria lograda\n");


	freeaddrinfo(serverInfo_memoria);

	return socketMemoria;
}

