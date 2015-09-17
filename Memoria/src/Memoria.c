/*
 ============================================================================
 Name        : Memoria.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Memoria
 ============================================================================
 */

#include "SharedLibs/libreriaCliente.h" //SharedLibs/Debug
#include "SharedLibs/libreriaServidor.h" //SharedLibs/Debug


#define BACKLOG 10
#define PACKAGESIZE 1024

const char *IPMEM="127.0.0.1";
const char *IPSWAP="127.0.0.1";
const char *PUERTOSWAP="9000";
const char *PUERTOMEM="8090";

int main(void) {
	puts("!!!Memoria!!!"); /* prints !!!Memoria!!! */


	//inicio cliente con el swap
	int socketSwap = crearCliente(IPSWAP, PUERTOSWAP);//conecta con el swap


	//inicio server
	int CPUSocket= crearServer(IPMEM, PUERTOMEM);

	//funcion que permite al programa ponerse a la espera de nuevas conexiones
	int L = listen(CPUSocket, BACKLOG);
	if (L==-1)
		perror("LISTEN");


	//Estructura que tendra los datos de la conexion del cliente
	conexiones.socket_escucha = CPUSocket;
	conexiones.tamanio_direccion = sizeof(conexiones.direccion);
	pthread_t hilo_escuchas;
	if( pthread_create(&hilo_escuchas, NULL, (void*)escuchar,&conexiones) < 0)
		puts("Error HILO ESCUCHAS!");

	puts("ESPERANDO CPU....\n");
	while(conexiones.CPU[0]==0);

	//fin server

	char message[PACKAGESIZE] = "lalala";
	int status;
	printf("CPU conectada. Esperando instrucciones:\n");
	//Envio de instrucciones
	int i;
	while(strcmp(message,"salir\n") !=0)
	{
		for(i=0;i<5;i++)
		{
			status = recv(conexiones.CPU[i], (void*) message, PACKAGESIZE, 0);
			if (status != 0){
				printf("RECIBIDO! =D\n%s", message);
				send(socketSwap, message, strlen(message) + 1, 0);
			}
			else{
				puts("conexion perdida! =(");
				break;
			}
		}
	}


	return EXIT_SUCCESS;
}
