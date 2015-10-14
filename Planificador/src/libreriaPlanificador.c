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
	char ruta[20]; //la ruta es de 20?
	puts("entre a correr");

	//pido la ruta del archivo
	puts("Ingrese el nombre del archivo que desea correr:");
	fflush(stdin);
	fgets(ruta, 20, stdin);

	//imprimo tamaño de la ruta ingresada
	printf("Tamaño ruta:%d\nPath:%s\n", sizeof(ruta),ruta);
	//char *path = (char*)malloc(sizeof(string_substring_from(message, 7)));

	//Se crea el PCB y se lo pone en la cola de ready
	sem_post(&semConsola); // debe ir arriba del procesarPCB para que se aproveche el paralelismo
	queue_push(cola_ready, procesarPCB(ruta));

	puts("PCB procesado y encolado\n");

	//sem_wait(&semSalir); //es para pruebas
	//free(path);
}

//Funcion despachador, que sacar un proceso de la cola de ready y enviarlo a la CPU para su ejecucion.
void dispatcher(t_queue *cola_ready)
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
	int I = 0;
	while((conexiones.CPUS[I].enUso) && (I <= MAX_CPUS))
	{
		I++;
	}
	
	if(!conexiones.CPUS[I].enUso){
		//bloqueo la cpu
		conexiones.CPUS[I].enUso = true;
		//CPU DISPONIBLE  saco de la cola y envio msj
		t_pcb *pcb = queue_pop(cola_ready);
		t_msj *msj = malloc(sizeof(t_msj));
		*msj = preparoMSJ(pcb);
		printf("Instruccion enviada:%d\nTamaño:%d\nSocket:%d\n",msj->headMSJ.tipo_ejecucion,msj->headMSJ.tamanio_msj,conexiones.CPUS[I].socket);
		send(conexiones.CPUS[I].socket, msj, sizeof(t_msj),0);
		free(msj);

		sem_post(semProduccionMsjs);
		puts("Mensaje enviado\n");
		log_info(logger,"Comienzo ejecucion PID: %d Nombre: %s", pcb->PID, pcb->ruta);

		//RECIBE RESPUESTA
		//esta bien que reciba la respuesta el despachador???
		//porque sino se bloquea habria que poner semaforos creo
		/*flag recibi = false;
		recv(conexiones.CPUS[I].socket, &recibi, sizeof(flag),0);
		if(recibi)
			puts("RECIBI OK");
		else
			puts("RECIBI FAIL");*/

		//libero la cpu
		conexiones.CPUS[I].enUso = false;
	}else{
		//sino hay cpu disponible no hago nada
		puts("CPU ocupada");

		//deberia volver a ejecutar el despachador o mandar algun aviso que esta ocupado???

	}

}

//Funcion que permite procesar el PCB creado a partir del comando correr PATH
t_pcb* procesarPCB(char *path)
{
	//inicializo las variables y reservo memoria (hay que ver donde hacer los free)
	t_pcb* pcb = (t_pcb*)malloc(sizeof(t_pcb));
	pcb->ruta = (char*)malloc(sizeof(path));//MODIFICAR

	//armo PCB y msj para enviar al CPU
	pcb->PID = max_PID +1;
	pcb->instructionPointer = 0;
	pcb->numInstrucciones = 0;
	pcb->prioridad=0;
	pcb->permisos=0;
	strcpy(pcb->ruta, path);
	
	return pcb;
}

t_msj preparoMSJ(t_pcb *pcb){
	t_msj msj;
	msj.pcbMSJ.ruta = malloc(sizeof(pcb->ruta));
	msj.headMSJ.tipo_ejecucion = 1;	//Orden de envio de pcb
	msj.headMSJ.tamanio_msj = sizeof(t_pcb);
	msj.pcbMSJ = *pcb;
	
	return msj;
}
