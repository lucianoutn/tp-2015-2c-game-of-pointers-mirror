/*
 ============================================================================
 Name        : Planificador.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Planificador
 ============================================================================
 */

#include "libreriaServidor.h"


#define BACKLOG 10
#define PACKAGESIZE 1024



int main() {
	puts("!!!Planificador!!!"); /* prints !!!Planificador!!! */

	const char *IP= "127.0.0.1";
	const char *PUERTOPLANIFICADOR= "8080";


	int CPUSocket= crearSocket(IP, PUERTOPLANIFICADOR);

	//funcion que permite al programa ponerse a la espera de nuevas conexiones
	int L = listen(CPUSocket, BACKLOG);
	if (L == -1)
		perror("LISTEN");


	//Estructura que tendra los datos de la conexion del cliente
	conexiones.socket_escucha = CPUSocket;
	conexiones.tamanio_direccion = sizeof(conexiones.direccion);
	pthread_t hilo_escuchas;
	if(pthread_create(&hilo_escuchas, NULL, escuchar,&conexiones)<0)
		perror("Error HILO ESCUCHAS!");

	puts("ESPERANDO CONEXIONES....\n");
	while(conexiones.CPU[0] == 0){
	};


	int socket_instrucciones, caracter, enviar;
	char message[PACKAGESIZE];

	while(strcmp(message,"salir\n") !=0)
	{
		puts("Elija CPU: ¡¡¡¡SOLO NUMEROS!!!\n");
		int j = 1;
		while ( j < 6) {
			printf("CPU n°:%d, puerto: %d\n",j,conexiones.CPU[j-1]);
			j++;
		}
		scanf("%d", &caracter);
		enviar =1;
		switch (caracter) {
			case 1:
				socket_instrucciones = conexiones.CPU[0];
				break;
			case 2:
				socket_instrucciones = conexiones.CPU[1];
				break;
			case 3: socket_instrucciones = conexiones.CPU[2];
				break;
			case 4:
				socket_instrucciones = conexiones.CPU[3];
				break;
			case 5:
				socket_instrucciones = conexiones.CPU[4];
				break;
			default:
				{puts("CPU NO VALIDA!"); enviar=0; caracter=0;};
				break;
		}

		printf("Ya puede enviar instrucciones.\nEscriba 'correr programa' para enviar una señal al CPU\n'cpu' para cambiar de CPU\n'salir' para cerrar los procesos\n");
		while(enviar){
		fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message,"cpu\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
		if (!strcmp(message,"correr programa\n")) send(socket_instrucciones, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
		if (!strcmp(message,"salir\n")){ send(socket_instrucciones, message, strlen(message) + 1, 0); break;};
		}

	}

		close(conexiones.socket_escucha);
		close(CPUSocket);

	return EXIT_SUCCESS;
}

