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
#include "consola.h"





int main() {
	semSalir.__align =0;
	semProduccionMsjs = sem_open("semPlani", O_CREAT, 0644, 0);//inicializo sem prod-consum, el 0_creat es para evitar q se inicialize en el otro proceso
	semRespuestaCpu = sem_open("semCPU", 0);
	PID_actual=0;      //inicializo numero de pid (yo le cambiaria el nombre a PID_actual)
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

	puts("ESPERANDO CONEXIONES....\n");
	sem_wait(&semEsperaCPU); //semaforo espera conexiones


	//Se crea la cola de ready's
	cola_ready = queue_create();
	//Se crea la cola de bloqueados
	cola_block = queue_create();
	//creo la lista de TODOS los pcb
	lstPcbs= list_create();


	//crear hilo de consola para que quede a la escucha de comandos por consola para el planificador

	//consola(); //sin hilo

	pthread_t hilo_consola, hilo_dispatcher;
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);
	//creo hilo despachador aca?
	pthread_create(&hilo_dispatcher, NULL, (void*)dispatcher, &cola_ready);


	int recivoOrden=1;
	while (recivoOrden)
	{
		sem_wait(&ordenIngresada);
		switch (orden)
		{
			case 0: //orden correr
			{
				encolar(lstPcbs, cola_ready);

				if	(!strcmp(miContexto.algoritmoPlanificacion, "FIFO")){ //por FIFO
					puts("FIFO");

					//AGREGAR SEMAFORO PRODUCTOR CONSUMIDOR PARA EL DISPATCHER
					sem_post(&semEnvioPcb);



					//dispatcher(cola_ready);
					/*
					 *
					 *si se acaba el quanto de tiempo vuelvo a encolar
					 *si hay instruccion de entrada-salida (recibe orden del cpu) agrego el proceso a
				  	  cola_bloqueados
				  	 *cuando termina de ejecutar entrada-salida recibe interrupcion y saco de la
				  	  cola de bloqueados y meto en la cola de ready
					 *si termina de procesar el proceso no encolo y mato el pcb
					 */

				}
				else{
				//por RoundRobin
					puts("RoundRobin");
				}
				break;
			}
			case 1: //orden salir
			{
				recivoOrden=0;
				break;
			}
		}
	}


	//pthread_join(hilo_consola, NULL); no habiliten este join, usamos semaforos.lucho


	//cierra los sockets

	sem_wait(&semSalir);
	puts("FINALIZANDO PROGRAMA\n");
	close(conexiones.socket_escucha);
	int i=0;
	while(i<MAX_CPUS)
	{
		close(conexiones.CPUS[i++].socket);
	}
	

	return EXIT_SUCCESS;
}


