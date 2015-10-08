/*
 * libreriaPlanificador.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#include "libreriaPlanificador.h"



//Funcion que permite cargar el archivo de configuracion en el programa.
void traigoContexto()
{
 // LEVANTO EL ARCHIVO CONFIG Y VERIFICO QUE LO HAYA HECHO CORRECTAMENTE /
 t_config * config_planificador = config_create("resources/config.cfg");

 if( config_planificador == NULL )
 {
  puts("No se cargo el archivo de configuracion");
  abort();
 }

 // OBTENGO CONFIGURACION DEL CONFIG /
 miContexto.puertoEscucha = config_get_string_value(config_planificador, "PUERTO_ESCUCHA" );
 miContexto.algoritmoPlanificacion = config_get_string_value(config_planificador, "ALGORITMO_PLANIFICACION" );
}

//Funcion que permite procesar el PCB creado a partir del comando correr PATH
void procesarPCB(t_queue* cola_ready, char * path, int socketCPU)
{
	CPUenUso=true;
	/*enviar(PCB) a la CPU
	 *esperar a que la CPU termine de procesar
	 *recibe respuesta de la CPU loguea y termina
	 */

	//inicializo las variables y reservo memoria (hay que ver donde hacer los free)
	t_msj msj;
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->ruta = malloc(20);

	//header
	msj.headMSJ.tipo_ejecucion = 1;
	//tamaño a recibir igual tamaño total del msj menos el header que lo recibe de una
	msj.headMSJ.tamanio_msj = (sizeof(t_msj) - sizeof(t_headcpu));

	//copio el path
	int x;
	for (x = 0; x < 20; x++) {
		msj.ruta[x] = path[x];
		pcb->ruta[x] = path[x];
	}

	//armo PCB y msj para enviar al CPU
	pcb->instructionPointer = 0;
	msj.instructionPointer = pcb->instructionPointer;
	pcb->numInstrucciones = 0;
	msj.numInstrucciones = pcb->numInstrucciones;
	pcb->prioridad=0;
	msj.prioridad = pcb->prioridad;
	pcb->permisos=0;
	msj.permisos = pcb->permisos;
	pcb->PID = max_PID +1;
	msj.PID = pcb->PID;
	max_PID = pcb->PID; // ¿aca no habria q preguntar si es menor primero?? (lucho)

	/*ENVIO DE MSJS A LA CPU
	 *	pcb->PID=contador;
	 *	pcb->instructionPointer=0
	 *	pcb->numInstrucciones=0;
	 *	pcb->prioridad=0;
	 *	pcb->permisos=0;
	 *	pcb->ruta=(char*)malloc(sizeof(path));
	 *	pcb.ruta=path;
	 *	free(path);
	 *	send(socketCPU, pcb, sizeof(t_pcb));
	 */
	/*RECIBE MSJS LA CPU
	 * 	t_pcb pcb=(t_pcb*)malloc(sizeof(t_pcb));
	 * 	recibo LA DIRECCION DE MEMORIA del pcb
	 * 	pcb=msj;
	 * 	imprimo a ver que onda
	 */

	//creo q aca estaria bueno q pregunte por el algoritmo de planificacion configurado ¿?(lucho)
	//encolo a la cola de ready
	//queue_push(cola_ready, &pcb);
	//le envio a la CPU el t_headcpu y el PCB

	CPUenUso=false;

	send(socketCPU, &msj,sizeof(t_msj),0);
	puts("Mensaje enviado\n");
	log_info(logger,"Comienzo ejecucion PID: %d Nombre: %s", pcb->PID, pcb->ruta);
}


void iniciarPlanificador(t_queue* cola_ready)
{
	int primeraVez = 1;
	int enviar =1 ;
	int cpuElegida;
	int socketCPU = conexiones.CPU[0];
	char* message=malloc(20);
	//t_pcb *PCB=(t_pcb*)malloc(sizeof(t_pcb));
	//PCB->ruta=path;
	// encolar(inicio, PCB);
	//controla que el usuario no quiera salir
	while(strcmp(message,"menu\n") !=0)
	{
		if(!primeraVez)
		{
			//Muestra las conexiones con las CPUS disponibles
			puts("Elija CPU: ¡¡¡SOLO NUMEROS!!!\n");
			int j = 0;
			while ( j < 1)
			{
				printf("CPU n°:%d, puerto: %d\n",j+1,conexiones.CPU[j]);
				j++;
			}
			//Permite elegir la conexion con el CPU deseado
			scanf("%d", &cpuElegida);
			socketCPU = conexiones.CPU[cpuElegida];
		}

		//Permite el envio de paquetes, dependiendo si la opcion elegida es valida
		printf("COMANDOS VALIDOS:\n"
			   "'correr PATH':      Para correr un proceso en el CPU (la primera por defecto)\n"
			   "'cpu':              Para cambiar de CPU\n"
			   "'menu':             Para volver al menu principal\n");
		while(enviar)
		{
			fflush(stdin);
			fgets(message, 20, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
			if (!strcmp(message,"cpu\n"))
			{
				primeraVez = 0;
				//enviar = 0;			// Chequeo que el usuario no quiera salir
				break;
				printf("Comando no implementado");
			}
			//MODIFICAR QUE PASA SI LA CPU ESTA EN USO
			/*
			if(CPUenUso)
			{
				//creo el hilo con la funcion procesarPCB (PCB);
				if(pthread_create(&hCPU1,NULL,procesarPCB,&PCB)<0)
					perror("Error HILO CPU!");
			}
			*/

			/*
			 * Protocolo de mensajes Planificador -CPU
			 * Para poder entender los distintos tipos de mensajes que se envia, mandamos primero
			 * un header t_headcpu.
			 * tipo_ejecucion: 0 - salir
			 * 				   1 - correr programa
			 * 				   	   envia PCB
			 * tamanio_mensaje: tamanio del char * o del PCB
			 */
			if (!strcmp(string_substring(message,0,7),"correr "))
			{
				primeraVez=1;
				char *path = (char*)malloc(sizeof(string_substring_from(message, 7)));
				//Tomo el path
				path = string_substring_from(message, 7); //inclye el \n
				//PROCESO EL PCB
				procesarPCB(cola_ready, path,socketCPU);
				puts("PCB Procesado\n");
				//sem_wait(&semSalir); //es para pruebas
				free(path);
				break;
			}
			if (!strcmp(message,"menu\n"))
			{
				//Salgo
				break;
			}
		}
	}
}
