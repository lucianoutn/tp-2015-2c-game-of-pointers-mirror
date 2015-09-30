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
	puts("!!!!Planificador!!!!"); /* prints !!!Planificador!!! */

	/* El planificador en alguna seccion especifica del programa cuando reciba el comando corre PATH
	 * por consola debera abrir el archivo "mCod" en la ruta indicada, leerlo y enviar las instrucciones del
	 * archivo a la CPU que este disponible en el momento. Esto se debe poder repetir indefinidas veces.
	 */

	/* Al abrir un archivo mCod, el planificador almacenara la informacion del nuevo proceso mProc en
	 * un nuevo nodo de la cola PCB. Se agregara este nodo a la cola, la cual en un principio es una
	 * cola de algoritmo FIFO
	 */

	/* El planificador debera seleccionar los elementos que esten primeros en la cola de ejecucion y
	 * enviar las instrucciones a una de las CPU disponibles (ya veremos como la elige) para procesar
	 */

	// El planificador debe recibir los resultados de la CPU.

	/*
	 * Funcion que crea un socket nuevo y lo prepara para escuchar conexiones entrantes a travez del puerto PUERTO
	 * y lo almacena en la variable conexiones del tipo struct Conexiones
	 */
	traigoContexto();

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
	pthread_t hilo_conexiones, hilo_consola;
	if(pthread_create(&hilo_conexiones, NULL, (void*)escuchar,&conexiones)<0)
		perror("Error HILO ESCUCHAS!");

	/*
	 * REEMPLAZAR MAS ADELANTE (SINCRONIZACION)
	 */
	puts("ESPERANDO CONEXIONES....\n");
	while(conexiones.CPU[0] <= 0){
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

	/* Cuando reciba el comando correr PATH, se creara un nuevo hilo en donde se crea el PCB del nuevo proceso. El
	hilo debe ejecutar la siguiente rutina:
	t_pcb PCB;
	PCB.PID=PID_actual
	PCB.instructionPointer=0
	char **instrucciones= (char**)malloc(sizeof(leermCod(PATH, PCB.numInstrucciones));
	instrucciones = (leermCod(PATH, PCB.numInstrucciones);
	
	
	(otra funcion)
	while(!strcmp(instruccion[num],"fin")) //envia hasta que llegue a la ultima instrucciones
	{
		send(payload) donde mensaje es una estructura con el payload y el mensaje es el string al que
		send(instrucciones[I]); 
		apunta el puntero instruccion[instructionPointer];
		instructionPointer++;
	}
	si llega uno y:
	if(!rafaga de CPU se acabe)
	{
		encolar(PCB) el que llega
		PCB.prioridad=1;

	}

	el proceso entra a rafaga de I/O
	se guarda el numero de instruccion, luego se retomara la ejecucion enviando desde la instruccion actual


	 */

	return EXIT_SUCCESS;
}


