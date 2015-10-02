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




int main() {
	semSalir.__align =0;
	puts("!!!!Planificador!!!!"); /* prints !!!Planificador!!! */

	// El planificador debe recibir los resultados de la CPU.


	traigoContexto(); //levanta el archivo de configuracion

	/*
	 * Funcion que crea un socket nuevo y lo prepara para escuchar conexiones entrantes a travez del puerto PUERTO
	 * y lo almacena en la variable conexiones del tipo struct Conexiones
	 */

	conexiones.socket_escucha=crearServer(miContexto.puertoEscucha);

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
	pthread_t hilo_conexiones;
	if(pthread_create(&hilo_conexiones, NULL, (void*)escuchar,&conexiones)<0)
		perror("Error HILO ESCUCHAS!");

	/*
	 * REEMPLAZAR MAS ADELANTE (SINCRONIZACION)
	 */
	puts("ESPERANDO CONEXIONES....\n");
	sem_wait(&semEsperaCPU); //semaforo espera conexiones


	//Se crea la cola de ready
	t_pcb *inicio=(t_pcb*)malloc(sizeof(t_pcb));
	//llamo a la funcion consola para activar la consola y se le envia el principio de la cola

	consola(inicio);

	/*
		pthread_create(&hilo_consola, NULL, (void*)consola, NULL);

		pthread_join(hilo_consola, NULL);
	*/



	//cierra los sockets
	puts("FINALIZANDO PROGRAMA\n");
	sem_wait(&semSalir);
	close(conexiones.socket_escucha);
	int i=0;
	while(i<5)
	{
		close(conexiones.CPU[i++]);
	}
	/* Cuando reciba el comando correr PATH, se creara un nuevo hilo en donde se crea el PCB del nuevo proceso. El
		hilo debe ejecutar la siguiente rutina:
		t_pcb PCB;
		PCB.ruta=path;
		envia ruta del PCB a la CPU, la cual termina de completar el PCB y empieza a ejecutar la rafaga
		de CPU.
	
	(otra funcion)
	
	si llega uno y pregunta si ya termino de ejecutar el hilo (pq en este caso no hay cuanto de tiempo
	por lo que cnd se le manda el archivo a la CPU ejecutar hasta que termina)
	if(no termino hilo)
	{
		t_pcb PCB;
		PCB.ruta=path;
		encolar(inicio, PCB) el que llega
	}

	cuando se libera la CPU
	desencolar(inicio);
	envia ruta del PCB a la CPU.

	 */

	return EXIT_SUCCESS;
}


