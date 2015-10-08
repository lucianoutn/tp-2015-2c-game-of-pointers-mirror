/*
 ============================================================================
 Name        : Planificador.c
 Author      : Game of Pointers
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo - Planificador
 ============================================================================
 */

#include "libreriaPlanificador.h"


int main() {
	semSalir.__align =0;
	semProduccionMsjs = sem_open("semPlani", O_CREAT, 0644, 0);
	//mmap();
	//sem_init(&semProduccionMsjs, 1, 0); //la variable del medio es "0" para hilos y <>0 para procesos
	max_PID=0;      //inicializo numero de pid
	puts("!!!!Planificador!!!!"); /* prints !!!Planificador!!! */

	// El planificador debe recibir los resultados de la CPU.


	traigoContexto(); //levanta el archivo de configuracion
	creoLogger(0); //recive 0 para log solo x archivo| recive 1 para log x archivo y x pantalla
	log_info(logger, "Inicio Log PLANIFICADOR", NULL);

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


	//Se crea la cola de readys
	t_queue *  cola_ready = queue_create();
	//crear hilo de consola para que quede a la escucha de comandos por consola para el planificador

	consola(); //sin hilo
	/*
		pthread_create(&hilo_consola, NULL, (void*)consola, NULL);

		pthread_join(hilo_consola, NULL);
	*/

	//si entra el comando correr PATH desde el hilo consola:




	//cierra los sockets

	sem_wait(&semSalir);
	puts("FINALIZANDO PROGRAMA\n");
	close(conexiones.socket_escucha);
	int i=0;
	while(i<MAX_CPUS)
	{
		close(conexiones.CPU[i++]);
	}
	

	return EXIT_SUCCESS;
}


