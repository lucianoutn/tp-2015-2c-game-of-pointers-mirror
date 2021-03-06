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
	semProduccionMsjs = sem_open("semPlani", O_CREAT, 0666, 0);//inicializo sem prod-consum, el 0_creat es para evitar q se inicialize en el otro proceso

	PID_actual=1;      //inicializo numero de pid (yo le cambiaria el nombre a PID_actual)

	// El planificador debe recibir los resultados de la CPU.

	traigoContexto(); //levanta el archivo de configuracion
	creoLogger(1); //recive 0 para log solo x archivo| recive 1 para log x archivo y x pantalla
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

	// alojo memoria dinamicamente en tiempo de ejecucion
	conexiones.CPUS= (t_cpu*)malloc(sizeof(t_cpu) * ((miContexto.cantHilosCpus) +1));
	if (conexiones.CPUS == NULL) puts("ERROR MALLOC 1");
	pthread_t hilo_conexiones;
	if(pthread_create(&hilo_conexiones, NULL, (void*)escuchar,&conexiones)<0)
		perror("Error HILO ESCUCHAS!");
	conexiones.CPUS[0].enUso = true;

	puts("PLANIFICADOR!\nESPERANDO CONEXIONES....\n\n\n");
	sem_wait(&semEsperaCPU); //semaforo espera conexiones


	//Se crea la cola de ready's
	cola_ready = queue_create();
	cola_block = queue_create();
	//creo la lista de TODOS los pcb
	lstPcbs= list_create();


	log_info(logger, "Conexion con la CPU establecida.", NULL);
	log_info(logger, "Cantidad de CPUS conectadas: %d.", miContexto.cantHilosCpus);
	//El siguiente if pisa el contexto del quantum, para q luego con -1 sea ignorado en caso FIFO
	if	(!strcmp(miContexto.algoritmoPlanificacion, "FIFO")){ 		//por FIFO
		log_info(logger, "Algoritmo seleccionado: FIFO");
		miContexto.quantum = -1;
	}else{				//por RoundRobin
		log_info(logger, "Algoritmo seleccionado: RoundRobin con un Quantum de: %d",miContexto.quantum);
	}
	//sleep(3); //estetico


	//crear hilo de consola para que quede a la escucha de comandos por consola para el planificador
	pthread_t hilo_consola, hilo_dispatcher, hilo_bloqueados;
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);
	//creo hilo despachador
	pthread_create(&hilo_dispatcher, NULL, (void*)dispatcher, NULL);
	//creo hilo bloqueados
	pthread_create(&hilo_bloqueados, NULL, (void*)bloqueados, NULL);

	int recivoOrden=1;
	while (recivoOrden)		//controla que no se salga desde la Consola()
	{
		sem_wait(&ordenIngresada);	//necesario para no tener espera activa
		switch (orden)
		{
			case 0: //orden correr
			{
				encolar(lstPcbs, cola_ready);
				break;
			}
			case 1: //orden salir
			{
				recivoOrden=0;
				pthread_cancel (hilo_dispatcher);
				break;
			}
			case 2: //comando cpu
			{
				//FALTA TERMINAR DE IMPLEMENTAR
				printf("\nUSO DE LA CPU\n\n");
				int usoCPU =0, nro;
				send(conexiones.CPUS[0].socket, &usoCPU, sizeof(int),0);
				for(nro=1; nro<=miContexto.cantHilosCpus;nro++)
				{
					recv(conexiones.CPUS[0].socket, &usoCPU, sizeof(int),0);
					printf("CPU %d: %d%\n",nro,usoCPU);
				}
				sem_post(&semConsola); //vuelvo a habilitar la consola
				break;
			}
		}
	}


	//pthread_join(hilo_consola, NULL); no habiliten este join, usamos semaforos.lucho
	//test

	//cierra los sockets y libero memoria

	sem_wait(&semSalir);	//no esta de mas este semaforo?
	log_info(logger, "FINALIZANDO PROGRAMA");
	//pthread_join(hilo_dispatcher, NULL);
	//list_iterate(lstPcbs,(void*)free); //mapeo la funsion imprimePS en cada nodo de la lista
	close(conexiones.socket_escucha);
	int i=1;
	while(i<=miContexto.cantHilosCpus)
	{
		close(conexiones.CPUS[i++].socket);
	}
	free(conexiones.CPUS);

	

	return EXIT_SUCCESS;
}


