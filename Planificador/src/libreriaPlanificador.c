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


void iniciarPlanificador(t_queue* cola_ready)
{
	//int primeraVez = 1;
	//int cpuElegida;
	/*
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
	*/	
	//primeraVez=1;
	
	puts("entre a correr");
	//pido la ruta del archivo
	puts("Ingrese el nombre del archivo que desea correr:\n");
	fflush(stdin);
	fgest(ruta, 20, stdin);
	//imprimo tamaño de la ruta ingresada
	printf("%d", sizeof(ruta));
	//char *path = (char*)malloc(sizeof(string_substring_from(message, 7)));
	//Se crea el PCB y se lo pone en la cola de ready
	queue_push(cola_ready, procesarPCB(ruta));
	puts("PCB procesado y encolado\n");
	sem_post(&semConsola); // debe ir arriba del procesarPCB para que se aproveche el paralelismo

	//sem_wait(&semSalir); //es para pruebas
	free(path);
	break;
	}
	//puts("ya sali del while");//prueba
}

//Funcion despachador, que sacar un proceso de la cola de ready y enviarlo a la CPU para su ejecucion.
void dispatcher(t_pcb *cola_ready)
{
	/*
	 * Protocolo de mensajes Planificador -CPU
	 * Para poder entender los distintos tipos de mensajes que se envia, mandamos primero
	 * un header t_headcpu.
	 * tipo_ejecucion: 0 - salir
	 *  1 - correr programa
	 * 				   envia PCB
	 * tamanio_mensaje: tamanio del char * o del PCB
	 */
	//busca la primer CPU que no este en uso
	/*
	while(!CPUenUso[I])
	{
		I++;
	}
	//¿Si no hay CPU disponible?
	*/
	//send(cola_ready, conexiones.CPU[I]); //envia el primer elemento de la cola
	CPUenUSO[I]=false; //pone la CPU ocupada hasta que reciba los resultados de la CPU.
	//ver en donde pongo la CPU como no ucupada.
	//queue_pop(t_queue *cola_ready); //quita el primer elemento de la cola a la CPU disponible
	
}

//Funcion que permite procesar el PCB creado a partir del comando correr PATH
t_pcb* procesarPCB(char *path)
{
	//inicializo las variables y reservo memoria (hay que ver donde hacer los free)
	//t_msj msj;
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->ruta =(char*)malloc(sizeof(path);//MODIFICAR

	//header
	msj.headMSJ.tipo_ejecucion = 1;
	//tamaño a recibir igual tamaño total del msj menos el header que lo recibe de una
	msj.headMSJ.tamanio_msj = (sizeof(t_msj) - sizeof(t_headcpu));

	//copio el path

	//armo PCB y msj para enviar al CPU
	pcb->PID = max_PID +1;
	pcb->instructionPointer = 0;
	pcb->numInstrucciones = 0;
	pcb->prioridad=0;
	pcb->permisos=0;
	strcpy(pcb->ruta, path);
	
	
	//Comento esto, ya que esta funcion solo va a asignar los valores al PCB no enviarlos.
	/*
	msj.instructionPointer = pcb->instructionPointer;
	msj.numInstrucciones = pcb->numInstrucciones;
	msj.prioridad = pcb->prioridad;
	msj.permisos = pcb->permisos;
	msj.PID = pcb->PID;
	*/
	
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

	
	return pcb;
	/*
	esto lo tiene que hacer el despachador
	send(socketCPU, &msj,sizeof(t_msj),0);
	sem_post(semProduccionMsjs);
	puts("Mensaje enviado\n");
	log_info(logger,"Comienzo ejecucion PID: %d Nombre: %s", pcb->PID, pcb->ruta);
	*/
}

