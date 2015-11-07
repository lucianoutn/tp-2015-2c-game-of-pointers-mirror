/*
 * libreriaPlanificador.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#include "libreriaPlanificador.h"

key_t key_pcb=1235; //clave de la seccion memoria pcb
key_t key_ruta=3212; //cable de la seccion memoria ruta


flag turno=false; //Flag que permite ver si el pcb ejecuto por primera vez


//Funcion que permite cargar el archivo de configuracion en el programa.
void traigoContexto()
{
 // LEVANTO EL ARCHIVO CONFIG Y VERIFICO QUE LO HAYA HECHO CORRECTAMENTE /
 t_config * config_planificador = config_create("resources/config.cfg");
 t_config * configHilos = config_create("../CPU/resources/config.cfg");

 if( config_planificador == NULL )
 {
  puts("No se cargo el archivo de configuracion del panificador");
  abort();
 }

 if( configHilos == NULL )
  {
   puts("No se cargo el archivo de configuracion de la cantidad de hilos del cpu");
   abort();
  }

 // OBTENGO CONFIGURACION DEL CONFIG /
 miContexto.puertoEscucha = config_get_string_value(config_planificador, "PUERTO_ESCUCHA" );
 miContexto.algoritmoPlanificacion = config_get_string_value(config_planificador, "ALGORITMO_PLANIFICACION" );
 miContexto.quantum= config_get_int_value(config_planificador, "QUANTUM" );
 miContexto.cantHilosCpus = config_get_int_value(configHilos, "CANTIDAD_HILOS");
}


//Funcion encargada de acceptar nuevas peticiones de conexion
void *escuchar (struct Conexiones* conexion){
	int i =0;
	semEsperaCPU.__align =0; // inicializa semaforo

	while( i< miContexto.cantHilosCpus ) //hasta q recorra todos los hilos de cpus habilitados
	{
		//guarda las nuevas conexiones para acceder a ellas desde cualquier parte del codigo
		conexion->CPUS[i].socket = accept(conexion->socket_escucha, (struct sockaddr *) &conexion->direccion, &conexion->tamanio_direccion);
		if(conexion->CPUS[i].socket==-1)
		{
			perror("ACCEPT");	//control error
		}
		conexion->CPUS[i].enUso = false;
		sem_post(&semEsperaCPU); //avisa que hay 1 CPU disponible
		puts("NUEVO HILO ESCUCHA!\n");
		log_info(logger, "CPU %d conectado", i);
		i++;
	}

	return NULL;
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
	sem_post(&semEnvioPcb); //habilito el dispacher.(lo pongo aca para q sirva tanto para fifo como RR) lucho
	//sem_wait(&semSalir); //es para pruebas
	//free(path);
}

//Funcion despachador, que sacar un proceso de la cola de ready y enviarlo a la CPU para su ejecucion.
void dispatcher()
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

	pthread_t hilo_CPU[miContexto.cantHilosCpus];

	puts ("DISPATCHER");
	sem_init(&semCpuLibre,0,miContexto.cantHilosCpus);
	while(1){
		puts ("WHILE DISPATCHER");
		//espera que alguien libere alguna CPU y alguien quiera enviar un PCB
		sem_wait(&semEnvioPcb); //revisar si estos dos semaforos no estan siempre juntos y se comprotan igual. lucho
		sem_wait(&semCpuLibre);//revisar si estos dos semaforos no estan siempre juntos y se comprotan igual. lucho
		puts ("PASO LOS SEMAFOROS");
		printf("cant hilos cpu: %d\n", miContexto.cantHilosCpus);
		//busca la primer CPU que no este en uso
		int I = 0;
		while((conexiones.CPUS[I].enUso) && (I < miContexto.cantHilosCpus)){
			I++;
		}

		//Crea un hilo por cada CPU
		if(pthread_create(&hilo_CPU[I],NULL,(void*)enviaACpu,&conexiones.CPUS[I])<0)
		perror("Error HILO CPU!");
	}

}

void enviaACpu(t_cpu CPU)
{	puts ("ENVIA A CPU");
	//bloqueo la cpu
	CPU.enUso = true;
	//CPU DISPONIBLE  saco de la cola y envio msj
	t_pcb *pcb = queue_pop(cola_ready);
	//chequeo el flag FINALIZAR. si esta prendido le pogno el IP al final, para cuando vuelva a ejecutar finalice. lucho
	if (pcb->finalizar) pcb->instructionPointer = pcb->numInstrucciones;

	//cambio estado de PCB a ejecutando
	pcb->estado=2;

	if(turno==false) //si es la primera vez que ejecuta
	{
		t_headcpu *header = malloc(sizeof(t_headcpu));
		preparoHeader(header);
		send(CPU.socket, header, sizeof(t_headcpu),0);
		turno=true;
	}
	else
	{
		send(CPU.socket, &turno, sizeof(flag), 0);
	}


	printf("mande un quantum de: %d\n", pcb->quantum); //teste
	//Envio el header

	puts("PCB enviado a la CPU para procesamiento\n");

	sem_post(semProduccionMsjs);

	log_info(logger,"Comienzo ejecucion PID: %d Nombre: %s", pcb->PID, pcb->ruta);

	//ESPERO RESPUESTA CON RCV
	flag termino=false;
	recv(CPU.socket, &termino, sizeof(flag),0);		//espero recibir la respuesta
	if(termino)	//Controlo que haya llegado bien
		puts("***test*** Volvio el pcb");


	//ESPERO RESPUESTA CON SEMAFORO
	/*
	 *La respuesta va a ser un signal que mande la CPU una vez que haya termiando de modificar el PCB
	 *compruebo el estado en el que esta el pcb
	 *hago switch y encolo o desencolo segun el estado!!
	 *
	 *(lucho) pero un semaforo no puede ser, xq no sabe CUAL es el pcb q se modifico. tendria q ser algo q le indique
	 *
	*/
	/*//estructura que hace el "wait" del semaforo:
	struct sembuf semOperacion;		//estructura q contiene la operacion sobre el semaforo
	semOperacion.sem_num = numeroCpu ;	//el indice del semaforo q quiero modificar
	semOperacion.sem_op = -1;  //este -1 seria el wait
	semOperacion.sem_flg = 0; //un flag siempre en0
	semop (semVCPU, &semOperacion, 1); //aplico la operacion sobre el semaforo
*/
	switch(pcb->estado)
	{
		case 1: //ready //solo aplica en rr //puede aplicar a fifo tmb cuando termina todo. hay q encolarlo una vez mas para q finalice solito
		{
			//si vuelve a estado ready es pq termino el cuanto
			//encolo denuevo en la cola de readys pq termino el cuanto pero no termino de usar la cpu
			queue_push(cola_ready, pcb);
			break;
		}
		case 3: //bloqueado
		{
			//queue_pop(cola_block, pcb);
			//espera tantos segundos
			//lo vuelve a meter en la cola de readys
			queue_push(cola_ready, pcb);
			break;
		}
		case 4: //finalizado
		{
			//libero todo el PCB!
			//lo saco de la cola NO DE LA LISTA
			//free(pcb); //no lo liberen aca xq dsp no los puedo ver como finalizados en el comando PS. luego podemos liberarlos desde la lista
			break;
		}
		default:
		{

		}
	}

	//sem_wait(semRespuestaCpu); preparo semaforos x si no bloquea al rcv. lucho

	/*flag recibi = false;

	recv(conexiones.CPUS[I].socket, &recibi, sizeof(flag),0);
	if(recibi)
		puts("RECIBI OK");
	else
		puts("RECIBI FAIL");*/

	//libero la cpu
	CPU.enUso = false;
	//sem_post(&semEnvioPcb); lo comento xq no se quiere enviar otro PCB cuando termina de ejecutar. Agus
	sem_post(&semCpuLibre);
}


//Funcion que permite procesar el PCB creado a partir del comando correr PATH
t_pcb* procesarPCB(char *path)
{
	long id_pcb = shmget(key_pcb, sizeof(t_pcb),(0666 | IPC_CREAT));//reservo espacio dentro de la seccion de memoria compartida
	t_pcb *pcb;
	pcb = (t_pcb*)shmat(id_pcb, 0, 0); //creo la variable y la asocio al segmento
	if (pcb == (t_pcb*)(-1))		//capturo error del shmat
		perror("shmat pcb");

	long id_ruta = shmget(key_ruta, sizeof(char*),(0666 | IPC_CREAT)); //reservo espacio dentro de la seccion de memoria compartida
	pcb->ruta = (char*)shmat(id_ruta, 0, 0); //creo la variable y la asocio al segmento
	if (pcb->ruta == (char*)(-1))		//capturo error del shmat
		perror("shmat ruta");

	//armo PCB
	pcb->PID= PID_actual+1;
	pcb->estado=0;
	pcb->instructionPointer = 0;
	pcb->numInstrucciones = 0;
	pcb->prioridad=0;
	pcb->permisos=0;
	strcpy(pcb->ruta, path);
	pcb->finalizar=false;
	pcb->quantum= miContexto.quantum;

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
