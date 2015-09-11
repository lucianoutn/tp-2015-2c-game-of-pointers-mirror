/*
 ============================================================================
 Name        : CPU.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Planificador
 ============================================================================
 */

#include "libreriaCliente.h"

/*
#define IP "127.0.0.1"
#define PUERTOPLANIFICADOR "8080"
#define PUERTOMEMORIA "8090"
*/
#define PACKAGESIZE 1024

const char *IP = "127.0.0.1";
const char *IP_MEMORIA = "127.0.0.1";  //agrego otra ip xq la mem esta en otra pc.Lucho
const char *PUERTOPLANIFICADOR = "8080";
const char *PUERTOMEMORIA = "8090";


//Inicio de funcion principal
int main()
{
	puts("!!!CPU!!"); /* prints !!!CPU!! */

	/*
	//Configuracion del socket
	struct addrinfo hints_planificador, hints_memoria; //estructura que almacena los datos de conexion de la CPU
	struct addrinfo *serverInfo_planificador, *serverInfo_memoria; //estructura que almacena los datos de conexion del Planificador
*/
/*
	memset(&hints_planificador, 0, sizeof(hints_planificador));
	hints_planificador.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints_planificador.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	serverInfo_memoria = serverInfo_planificador;
	hints_memoria = hints_planificador;
	getaddrinfo(IP, PUERTOPLANIFICADOR, &hints_planificador, &serverInfo_planificador);	// Carga en serverInfo los datos de la conexion

	//se crea un nuevo socket que se utilizara para la conexion con el Planificador
	int socketPlanificador, socketMemoria; //descriptor del socket servidor
	socketPlanificador = socket(serverInfo_planificador->ai_family, serverInfo_planificador->ai_socktype, serverInfo_planificador->ai_protocol);
	//se comprueba que el socket se creo correctamente
	if(socketPlanificador==-1)
		perror ("SOCKET PLANIFICADOR!");

	getaddrinfo(IP_MEMORIA ,PUERTOMEMORIA,&hints_memoria,&serverInfo_memoria);

	socketMemoria = socket(serverInfo_memoria->ai_family, serverInfo_memoria->ai_socktype, serverInfo_memoria->ai_protocol);
	if(socketMemoria==-1)
		perror("SOCKET MEMORIA!");
	*/

/*	//Conexion al servidor Planificador y a Memoria
	int C = connect(socketPlanificador, serverInfo_planificador->ai_addr, serverInfo_planificador->ai_addrlen);
	if (C==-1){
		perror ("CONNECT");
		return -1;
	}
	else
		printf ("Conexion con el Planificador lograda\n");

	int C2 = connect(socketMemoria, serverInfo_memoria->ai_addr, serverInfo_memoria->ai_addrlen);
	if (C2==-1){
		perror ("CONNECT");
		return -1;
	}
	else
		printf ("Conexion con la Memoria lograda\n");


	freeaddrinfo(serverInfo_planificador);
	freeaddrinfo(serverInfo_memoria);
*/

	int socketPlanificador = crearClientePlani(IP, PUERTOPLANIFICADOR);
	int socketMemoria = crearClienteMem(IP_MEMORIA, PUERTOMEMORIA);

//Recepcion de instrucciones

	char package[PACKAGESIZE];
	int status;		// Estructura que manjea el status de los recieve.
	printf("CPU conectada. Esperando instrucciones:\n");

	while(strcmp(package,"salir\n") !=0)
	{
		status = recv(socketPlanificador, (void*) package, PACKAGESIZE, 0);
		if (status != 0){
			printf("RECIBIDO! =D\n%s", package);
			send(socketMemoria, package, strlen(package) + 1, 0);
		}
		else{
			puts("conexion perdida! =(");
			break;
		}

	}

	close(socketPlanificador);
	close(socketMemoria);	// agrego el cierre del otro socket.lucho


	return EXIT_SUCCESS;
}
