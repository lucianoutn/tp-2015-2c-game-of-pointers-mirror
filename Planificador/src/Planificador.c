/*
 ============================================================================
 Name        : Planificador.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Planificador
 ============================================================================
 */

#include "planificador.h"

const char *IP = "127.0.0.1";
const char *PUERTO = "8080";

int main() {
	puts("!!!Planificador!!!"); /* prints !!!Planificador!!! */

	/*
	 * Funcion que crea un socket nuevo y lo prepara para escuchar conexiones entrantes a travez del puerto PUERTO
	 * y lo almacena en la variable conexiones del tipo struct Conexiones
	 */
	conexiones.socket_escucha=crearServer(IP,PUERTO);

	//funcion que permite al programa ponerse a la espera de nuevas conexiones
		int L = listen(conexiones.socket_escucha, BACKLOG);
		if (L == -1)
			perror("LISTEN");


		//Se calcula el tamaño de la direccion del cliente
		conexiones.tamanio_direccion = sizeof(conexiones.direccion);


	/*
	 * Se crea un hilo nuevo que se queda a la espera de nuevas conexiones del CPU
	 * y almacena los sockets de las nuevas conexiones en la variable conexiones.CPU[]
	 */
	pthread_t hilo_conexiones, hilo_consola;
	if(pthread_create(&hilo_conexiones, NULL, (void*)escuchar,&conexiones)<0)
		perror("Error HILO ESCUCHAS!");

	/*
	 * REEMPLAZAR MAS ADELANTE (SINCRONIZACION)
	 */
	puts("ESPERANDO CONEXIONES....\n");
	while(conexiones.CPU[4] <= 0){
	}

	//Crea un hilo para la consola y espera a que termine para finalizar el programa
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);
	pthread_join(hilo_consola, NULL);

	//cierra los sockets
	close(conexiones.socket_escucha);
	int i=0;
	while(i<5)
	{
		close(conexiones.CPU[i++]);
	}

	return EXIT_SUCCESS;
}


