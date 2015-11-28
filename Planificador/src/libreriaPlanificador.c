/*
 * libreriaPlanificador.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#include "libreriaPlanificador.h"

int key_pcb=1235; //clave de la seccion memoria pcb
int key_ruta=3212; //cable de la seccion memoria ruta



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
	int i =1;
	semEsperaCPU.__align =0; // inicializa semaforo

	//conexion para el comando cpu
	conexion->CPUS[0].socket = accept(conexion->socket_escucha, (struct sockaddr *) &conexion->direccion, &conexion->tamanio_direccion);

	while( i<= miContexto.cantHilosCpus ) //hasta q recorra todos los hilos de cpus habilitados
	{
		//guarda las nuevas conexiones para acceder a ellas desde cualquier parte del codigo
		conexion->CPUS[i].socket = accept(conexion->socket_escucha, (struct sockaddr *) &conexion->direccion, &conexion->tamanio_direccion);
		if(conexion->CPUS[i].socket==-1)
		{
			perror("ACCEPT");	//control error
		}
		conexiones.CPUS[i].numCpu = i;
		conexion->CPUS[i].enUso = false;
		sem_post(&semEsperaCPU); //avisa que hay 1 CPU disponible
		//puts("NUEVO HILO ESCUCHA!\n");
		log_info(logger, "CPU %d conectado", i);
		i++;
	}

	return NULL;
}


void encolar(t_list* lstPcbs, t_queue* cola_ready)
{
	char *ruta=(char*)malloc(sizeof(char) * 30);
	if (ruta == NULL) puts("ERROR MALLOC 2");
	//pido la ruta del archivo
	puts("Ingrese el nombre del archivo que desea correr:");
	fflush(stdin);
	fgets(ruta, 30, stdin); //30 es el tamaño maximo de la ruta a ingresar
	int tamanio=strlen(ruta);
	ruta[tamanio-1]='\0'; //agrego el caracter nulo al final de la ruta para que se indentifique como string.

	//Se crea el PCB y se lo pone en la cola de ready
	sem_post(&semConsola); // debe ir arriba del procesarPCB para que se aproveche el paralelismo. vuelve a habilitar ingreso de nueva orden
	t_pcb* pcb= procesarPCB(ruta);
	//inicializo metrica
	pcb->t_aux1=time(NULL);
	list_add(lstPcbs, pcb);
	//agrego el pcb a la cola de ready
	queue_push(cola_ready, pcb);


	//puts("PCB creado y encolado\n");
	sem_post(&semEnvioPcb); //habilito el dispacher.(lo pongo aca para q sirva tanto para fifo como RR) lucho
	//sem_wait(&semSalir); //es para pruebas
	free(ruta);
}

//Funcion despachador, que sacar un proceso de la cola de ready y enviarlo a la CPU para su ejecucion.
void dispatcher()
{
	pthread_t hilo_CPU[miContexto.cantHilosCpus];

	puts ("DISPATCHER ACTIVADO");
	sem_init(&semCpuLibre,0,miContexto.cantHilosCpus);
	while(1){
		//puts ("WHILE DISPATCHER");
		//espera que alguien libere alguna CPU y alguien quiera enviar un PCB
		sem_wait(&semEnvioPcb); //revisar si estos dos semaforos no estan siempre juntos y se comprotan igual. lucho
		sem_wait(&semCpuLibre);//revisar si estos dos semaforos no estan siempre juntos y se comprotan igual. lucho
		//puts ("PASO LOS SEMAFOROS");
		//printf("cant hilos cpu: %d\n", miContexto.cantHilosCpus);
		//busca la primer CPU que no este en uso
		int I = 1;
		while((conexiones.CPUS[I].enUso) && (I <= miContexto.cantHilosCpus)){
			I++;
		}
		//printf("CPU ELEGIDA: %d\n SOCKET ELEGIDO: %d\n",I, conexiones.CPUS[I].socket);
		//Crea un hilo por cada CPU
		if(pthread_create(&(hilo_CPU[I]),NULL,(void*)enviaACpu,&conexiones.CPUS[I])<0)
		perror("Error HILO CPU!");
	}

}

void enviaACpu(t_cpu *CPU)
{
	time_t inicio, fin;

	//printf("SOY LA CPU Nro: %d\n", CPU->numCpu);
	//bloqueo la cpu
	CPU->enUso = true;
	//CPU DISPONIBLE  saco de la cola y envio msj
	t_pcb *pcb = queue_pop(cola_ready);
	//finalizo metrica
	pcb->t_espera+= difftime(time(NULL), pcb->t_aux1);

	//chequeo el flag FINALIZAR. si esta prendido le pogno el IP al final, para cuando vuelva a ejecutar finalice. lucho
	if (pcb->finalizar) {pcb->instructionPointer = (pcb->numInstrucciones - 1); puts("QUIERO FINALIZAR");}

	//cambio estado de PCB a ejecutando
	pcb->estado=2;
	//inicializo la metrica de ejecucion
	inicio=time(NULL);
	//Seteo en quantum
	if(pcb->quantum==0){
		pcb->quantum=miContexto.quantum;
	}

	//if(pcb->quantum >= 0) printf("mande un quantum de: %d\n y el socket es: %d\n", pcb->quantum, CPU->socket); //teste

	t_headcpu *header = malloc(sizeof(t_headcpu));
	if (header == NULL) puts("ERROR MALLOC 3");
	preparoHeader(header, pcb->PID);

	//Envio el header
	send(CPU->socket, header, sizeof(t_headcpu),0);
	log_info(logger, "mProc %d <%s> Ejecutando en cpu: %d",pcb->PID,ruta(pcb->PID),CPU->numCpu);

	free(header);

	sem_post(semProduccionMsjs);

	//puts("PASO SEMAFORO");
	//ESPERO RESPUESTA CON RCV
	flag termino=false;
	recv(CPU->socket, &termino, sizeof(flag),0);		//espero recibir la respuesta
	if(!termino)	//Controlo que haya llegado bien
		log_info(logger, "mProc %d <%s> Fallo",pcb->PID,ruta(pcb->PID));

	//Finalizo el conteo y replico en la metrica
	fin=time(NULL);
	pcb->t_ejecucion+= difftime(fin, inicio);
	//libero la cpu
	CPU->enUso = false;
	sem_post(&semCpuLibre);

	switch(pcb->estado)
	{
		case 1: //ready //solo aplica en rr //puede aplicar a fifo tmb cuando termina todo. hay q encolarlo una vez mas para q finalice solito
		{
			//si vuelve a estado ready es pq termino el cuanto
			//encolo denuevo en la cola de readys pq termino el cuanto pero no termino de usar la cpu
			log_info(logger, "mProc %d <%s> Volvio a la cola",pcb->PID,ruta(pcb->PID));
			//inicializo metrica
			pcb->t_aux1=time(NULL);
			//Agrego a la cola de ready
			queue_push(cola_ready, pcb);
			sem_post(&semEnvioPcb);
			break;
		}
		case 3: //bloqueado
		{
			//inicializo la metrica
			pcb->t_aux2 =time(NULL);
			//bloqueo el pcb, lo pongo en la cola de bloqueados
			queue_push(cola_block, pcb);
			sem_post(&semBloqueados); //habilito el hilo de bloqueados
			break;
		}
		case 4: //finalizado
		{
			log_info(logger, "mProc %d <%s> Finalizo\n       Tiempo de ejecucion: %d - Tiempo de espera: %d - Tiempo de respuesta %d",pcb->PID,ruta(pcb->PID),pcb->t_ejecucion,pcb->t_espera,pcb->t_respuesta);
			//log_info(logger, "mProc %d <%s> Tiempo de ejecucion: %d",pcb->PID,ruta(pcb->PID), pcb->t_ejecucion);
			//log_info(logger, "mProc %d <%s> Tiempo de espera: %d",pcb->PID,ruta(pcb->PID), pcb->t_espera);
			//log_info(logger, "mProc %d <%s> Tiempo de respuesta %d",pcb->PID,ruta(pcb->PID), pcb->t_respuesta);
			//libero todo el PCB!
			//lo saco de la cola NO DE LA LISTA
			//free(pcb); //no lo liberen aca xq dsp no los puedo ver como finalizados en el comando PS. luego podemos liberarlos desde la lista
			break;
		}
		case 5: //fallo
		{
			log_info(logger, "mProc %d <%s> Fallo",pcb->PID,ruta(pcb->PID));
			break;
		}
		default:
		{

		}
	}


}

//Funcion para hacer el tratamiento de la cola de bloqueados
void bloqueados()
{
	while(1) //habilitado todo el tiempo // ojo con la espera activa
	{
		//sem productor-consumidor
		//saco el pcb de la cola
		sem_wait(&semBloqueados);
		t_pcb* pcb= queue_pop(cola_block);
		//lo bloqueo segun el tiempo indicado
		log_info(logger, "mProc %d En entrada-salida de tiempo: %d",pcb->PID,pcb->tiempo);
		usleep(pcb->tiempo * 1000000);
		//replico metricas
		pcb->t_respuesta+=difftime(time(NULL), pcb->t_aux2);
		//inicializo la metrica
		pcb->t_aux1=time(NULL);
		//actualizo el estado
		pcb->estado=1;
		//lo vuelvo a poner en la cola de ready
		queue_push(cola_ready, pcb);
		//aviso
		sem_post(&semEnvioPcb);
	}

}

//Funcion que permite procesar el PCB creado a partir del comando correr PATH
t_pcb* procesarPCB(char *path)
{
	long id_pcb = shmget((key_t)(PID_actual + key_pcb), sizeof(t_pcb),(0666 | IPC_CREAT)); //reservo espacio dentro de la seccion de memoria compartida
	t_pcb *pcb;
	pcb = (t_pcb*)shmat(id_pcb, 0, 0); //creo la variable y la asocio al segmento
	if (pcb == (t_pcb*)(-1))		//capturo error del shmat
		perror("shmat pcb");

	/*
	long id_ruta = shmget((key_t)(PID_actual + key_ruta), sizeof(char*),(0666 | IPC_CREAT)); //reservo espacio dentro de la seccion de memoria compartida
	pcb->ruta = (char*)shmat(id_ruta, 0, 0); //creo la variable y la asocio al segmento
	if (pcb->ruta == (char*)(-1))		//capturo error del shmat
		perror("shmat ruta");
*/
	//armo PCB
	pcb->PID= (PID_actual++);
	pcb->estado=0;
	pcb->instructionPointer = 0;
	pcb->numInstrucciones = 0;
	pcb->prioridad=0;
	pcb->permisos=0;
	pcb->ruta = ruta(pcb->PID);
	strcpy(pcb->ruta, path);
	pcb->finalizar=false;
	pcb->quantum= miContexto.quantum;

	pcb->t_ejecucion=0;
	pcb->t_espera=0;
	pcb->t_respuesta=0;

	return pcb;
}

char* ruta(int PID){
	char *ruta;
	long id_ruta = shmget((key_t)(PID + key_ruta), sizeof(char*),(0666 | IPC_CREAT)); //reservo espacio dentro de la seccion de memoria compartida
	ruta = (char*)shmat(id_ruta, 0, 0); //creo la variable y la asocio al segmento
	if (ruta == (char*)(-1))		//capturo error del shmat
		perror("shmat ruta");
	return ruta;
}

//CAMBIAR PARA QUE SOLO SEA EL HEADER
void preparoHeader(t_headcpu *header, int PID)
{
	header->tipo_ejecucion= 1;	//Orden de envio de pcb
	header->clave_pcb=(key_t)(PID + key_pcb);
	header->clave_ruta=(key_t)(PID + key_ruta);
	
}

char* estadoActual (int estado) //la uso para el comando PS del planificador.lucho
{

	if(estado==0)
	{
		return "Nuevo";
	}else if(estado==1)
	{
		return "Listo";
	}else if(estado==2)
	{
		return "Ejecutando";
	}else if(estado==3)
	{
		return "Bloqueado";
	}else if(estado==4)
	{
		return "Finalizado";
	}else
	{
		return "Fallo";
	}
}
