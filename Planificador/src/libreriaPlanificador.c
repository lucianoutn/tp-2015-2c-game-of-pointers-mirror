/*
 * libreriaPlanificador.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#include "libreriaPlanificador.h"

key_t key_pcb=1235; //clave de la seccion memoria pcb
key_t key_ruta=3212; //cable de la seccion memoria ruta



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
 miContexto.quantum= config_get_int_value(config_planificador, "QUANTUM" );
}


void encolar(t_list* lstPcbs, t_queue* cola_ready)
{
	char *ruta=(char*)malloc(sizeof(char));

	//pido la ruta del archivo
	puts("Ingrese el nombre del archivo que desea correr:");
	fflush(stdin);
	fgets(ruta, 20, stdin); //20 es el tamaño maximo de la ruta a ingresar
	int tamanio=strlen(ruta);
	ruta[tamanio-1]='\0'; //agrego el caracter nulo al final de la ruta para que se indentifique como string.

	//Se crea el PCB y se lo pone en la cola de ready
	sem_post(&semConsola); // debe ir arriba del procesarPCB para que se aproveche el paralelismo. vuelve a habilitar ingreso de nueva orden
	t_pcb* pcb= procesarPCB(ruta);
	list_add(lstPcbs, pcb);
	queue_push(cola_ready, pcb);

	puts("PCB creado y encolado\n");

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
	 * 				   1 - correr programa
	 * 				   envia PCB
	 * tamanio_mensaje: tamanio del char * o del PCB
	 */

	pthread_t hilo_CPU[MAX_CPUS];

	while(1){

		//espera que alguien libere alguna CPU y alguien quiera enviar un PCB
		sem_wait(&semEnvioPcb);
		sem_wait(&semCpuLibre);

		//busca la primer CPU que no este en uso
		int I = 0;
		while((conexiones.CPUS[I].enUso) && (I <= MAX_CPUS)){
			I++;
		}

		//Crea un hilo por cada CPU
		if(pthread_create(&hilo_CPU[I],NULL,(void*)enviaACpu,&conexiones.CPUS[I])<0)
		perror("Error HILO CPU!");
	}

}

void enviaACpu(t_cpu CPU)
{
	//bloqueo la cpu
	CPU.enUso = true;
	//CPU DISPONIBLE  saco de la cola y envio msj
	t_pcb *pcb = queue_pop(cola_ready);
	//chequeo el flag FINALIZAR. si esta prendido le pogno el IP al final, para cuando vuelva a ejecutar finalice. lucho
	if (pcb->finalizar) pcb->instructionPointer = pcb->numInstrucciones;

	t_headcpu *header = malloc(sizeof(t_headcpu));
	preparoHeader(header);
	//Envio el header
	send(CPU.socket, header, sizeof(t_headcpu),0);
	puts("PCB enviado a la CPU para procesamiento\n");

	sem_post(semProduccionMsjs);

	log_info(logger,"Comienzo ejecucion PID: %d Nombre: %s", pcb->PID, pcb->ruta);

	//ESPERO RESPUESTA CON SEMAFORO
	/*
	 *compruebo el estado
	 *hago switch y encolo o desencolo segun el estado!!
	 switch(estado)
	 {
		 case 1 //ready
		 case 3 //bloqueado
		 case 4 //finalizado
	 */


	//sem_wait(semRespuestaCpu); preparo semaforos x si no bloquea al rcv. lucho

	/*flag recibi = false;

	recv(conexiones.CPUS[I].socket, &recibi, sizeof(flag),0);
	if(recibi)
		puts("RECIBI OK");
	else
		puts("RECIBI FAIL");*/

	//libero la cpu
	CPU.enUso = false;
	sem_post(&semEnvioPcb);
	sem_post(&semCpuLibre);
}


//Funcion que permite procesar el PCB creado a partir del comando correr PATH
t_pcb* procesarPCB(char *path)
{
	long id_pcb = shmget(key_pcb, sizeof(t_pcb),(0666 | IPC_CREAT));//reservo espacio dentro de la seccion de memoria compartida
	t_pcb *pcb;
	pcb = (t_pcb*)shmat(id_pcb, NULL, 0); //creo la variable y la asocio al segmento
	if (pcb == (t_pcb*)(-1))		//capturo error del shmat
		perror("shmat");

	long id_ruta = shmget(key_ruta, sizeof(char*),(0666 | IPC_CREAT)); //reservo espacio dentro de la seccion de memoria compartida
	pcb->ruta = (char*)shmat(id_ruta, NULL, 0); //creo la variable y la asocio al segmento
	if (pcb->ruta == (char*)(-1))		//capturo error del shmat
		perror("shmat");

	//armo PCB
	pcb->PID= PID_actual+1;
	pcb->estado=0;
	pcb->instructionPointer = 0;
	pcb->numInstrucciones = 0;
	pcb->prioridad=0;
	pcb->permisos=0;
	strcpy(pcb->ruta, path);
	pcb->finalizar=false;

	return pcb;
}

//CAMBIAR PARA QUE SOLO SEA EL HEADER
void preparoHeader(t_headcpu *header)
{
	header->tipo_ejecucion= 1;	//Orden de envio de pcb
	header->clave_pcb=key_pcb;
	header->clave_ruta=key_ruta;
	key_pcb++;
	key_ruta++;
	
}

char* estadoActual (int estado)
{

	if(estado==0)
	{
		return "new";
	}
	else if(estado==1)
	{
		return "ready";
	}
	else if(estado==2)
	{
		return "executing";
	}
	else if(estado==3)
	{
		return "block";
	}
	else
	{
		return "finish";
	}
}
