/*
 * funcCPU.c
 *
 *  Created on: 30/9/2015
 *      Author: utnso
 */


#include "funcCPU.h"

#include <sys/ipc.h>
#include <sys/shm.h>

/* Cargo las configuraciones del archivo Config.cfg
 * y compruebo que no haya errores.
 */
void cargoArchivoConfiguracion()
{
 // LEVANTO EL ARCHIVO CONFIG Y VERIFICO QUE LO HAYA HECHO CORRECTAMENTE /
 t_config * config_cpu = config_create("resources/config.cfg");

 if( config_cpu == NULL )
 {
  puts("Error al abrir el archivo de configuraciones.");
  abort();
 }

 // OBTENGO CONFIGURACION DEL CONFIG /
 configuracion.ipPlanificador = config_get_string_value(config_cpu, "IP_PLANIFICADOR" );
 configuracion.puertoPlanificador = config_get_string_value(config_cpu, "PUERTO_PLANIFICADOR" );
 configuracion.ipMemoria = config_get_string_value(config_cpu, "IP_MEMORIA");
 configuracion.puertoMemoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
 configuracion.cantHilos = config_get_int_value(config_cpu, "CANTIDAD_HILOS");
 configuracion.retardo = config_get_int_value(config_cpu, "RETARDO");

 //free(config_cpu);

}

/* Crea el Header para enviar a la memoria
 * con los datos del PCB, ejecucion y n° de pagina
 */
void creoHeader(t_pcb * pcb, t_header* header, int ejecucion, int pagina)
{
	//LEO INSTRUCCION DEL ARCHIVO
	header->PID = pcb->PID;
	header->pagina_proceso = pagina;
	header->tamanio_msj = 0;
	header->type_ejecution = ejecucion;
	//ARMO HEADER SEGUN PROTOCOLO
}


/* Recibe una palabra y devuelve el valor
 * del tipo de instruccion correspondiente
 * 0=iniciar	1=escribir	2=iniciar
 * 3=finalizar	4=entrada-salida
 */
int palabraAValor(char *palabra)
{
	int valor;
	if(strcmp(palabra,"iniciar")==0){
		valor=2;
	}
	else if (strcmp(palabra,"leer")==0){
		valor=0;
	}
	else if (strcmp(palabra,"escribir")==0){
		valor=1;
	}
	else if (strcmp(palabra,"entrada-salida")==0){
		valor=4;
	}
	else if (strcmp(palabra,"finalizar")==0){
		valor=3;
	}
	else{
		puts("instruccion no valida");
	}
	return valor;
}

/* Recibe una instruccion y una variable para la pagina
 * y devuelve el numero del tipo de instruccion y la pagina
 */
int procesaInstruccion(char* instruccion, int *pagina, char* mensaje){

	//Inicializo variables y reservo memoria
	int I=0, J=0, K=0, valor;
	char *palabra=(char*) malloc(sizeof(char));
	char *aux=(char*) malloc(sizeof(char));
	char *texto=(char*) malloc(sizeof(char));

	//CONTROLO QUE NO SE TERMINE LA PALABRA
	while(instruccion[I]!= ' ')
	{
		if(instruccion[I]== '\0')	break;
		palabra[I]=instruccion[I];
		I++;
		palabra= (char*)realloc(palabra, (I+1)*sizeof(char));
	}
	palabra[I]='\0';
	valor = palabraAValor(palabra);

	I++;	//incremento para ver el valor de instruccion
	//CONTROLO QUE NO SE TERMINE LA INSTRUCCION
	while(instruccion[I] != ' ')
	{
		if(instruccion[I]=='\0') break;
		aux[J]=instruccion[I];
		aux= (char*)realloc(aux, (J+1)*sizeof(char));
		I++; J++;
	}
	//Convierto a int la cadena y guardo el valor para liberar memoria
	*pagina = atoi(aux);

	//I++;	//incremento para ver el texto a escribir
	//CONTROLO QUE NO SE TERMINE LA INSTRUCCION
	int H=1;
	I++;
	while(instruccion[I] != '\0')
	{
		if (instruccion[I] == '\"'){
			if (H==0){
				texto[K]='\0';
				break;
			}else{
				H--;
			}
		}else{
			texto[K]=instruccion[I];
			texto= (char*)realloc(texto, (K+1)*sizeof(char));
			K++;
		}
		I++;
	}
	strcpy(mensaje, texto);

	//Libero memoria
	free(palabra);
	free(aux);
	free(texto);

	//Retorno el valor del tipo de instruccion
	return valor;

}

/* Procesa los MSJ recibidos
 * y envia el MSJ correspondiente a la memoria
 */
void ejecutoPCB(int socketMemoria, int socketPlanificador, t_pcb *PCB){

	int pagina = 0;
	char *mensaje;
	//reservo espacio para el header
	t_header *header = malloc(sizeof(t_header));

	//flag para controlar la respuesta a las instrucciones por parte de la memoria
	flag recibi= false;
	//flag para avisar al planificador que cambie de estado
	flag cambio= true;

	//reservo espacio en la memoria para guardar todas las instrucciones del archivo mCod
	//primero leo para saber el numero de instrucciones
	//char **instrucciones= (char**)malloc(sizeof(char*));
	//guardo las intrucciones
	char **instrucciones = (leermCod(PCB->ruta, &PCB->numInstrucciones));

	//Itera hasta llegar a la ultima instruccion
	while(PCB->estado!=4 && PCB->estado !=3)
	{
		if(PCB->quantum==0)
		{
			PCB->estado=1;
			//señal al plani avisando que cambie de estado
			send(socketPlanificador, &cambio, sizeof(flag), 0);
			break;
		}
		//Switch que verifica el tipo de cada instruccion
		switch(procesaInstruccion(instrucciones[PCB->instructionPointer],&pagina,mensaje))
		{

			case 0: //leer

				puts("LEER");
				creoHeader(PCB,header,0,pagina); //PCB HEADER TIPOEJECUCION PAGINA
				//printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
				send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
				recv(socketMemoria, &recibi, sizeof(bool),0);		//espero recibir la respuesta
				usleep(configuracion.retardo); //retardo del cpu
				if(recibi)	//Controlo que haya llegado bien
					puts("Leido");
				else
					puts("NO Leido");
				break;

			case 1: //Escribir

				//HAY QUE AGREGAR EL CAMPO PARA EL MSJ Y MANDARLO
				puts("ESCRIBIR");
				printf("Mensaje recibido: <%s>\nTamaño: %d.\n",mensaje,strlen(mensaje));
				creoHeader(PCB,header,1,pagina); //PCB HEADER TIPOEJECUCION PAGINA
				header->tamanio_msj = strlen(mensaje);
				/*struct {
					t_header head;
					char *msj;
				}auxiliar;
				auxiliar.head = *header;
				auxiliar.msj = (char*)malloc(sizeof(char) * header->tamanio_msj);
				strcpy(auxiliar.msj,mensaje);
				send(socketMemoria, &auxiliar, sizeof(auxiliar), 0);*/
				send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
				send(socketMemoria, mensaje, header->tamanio_msj,0);	//envio el texto a excribir
				recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
				usleep(configuracion.retardo); //retardo del cpu
				if(recibi)	//Controlo que haya llegado bien
					puts("Recibi ok");
				else
				puts("Error");
				break;

			case 2://iniciar

				puts("INICIAR");
				creoHeader(PCB,header,2,pagina); //PCB HEADER TIPOEJECUCION PAGINA
				//printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
				send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
				recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
				usleep(configuracion.retardo); //retardo del cpu
				if(recibi)	//Controlo que haya llegado bien
					puts("Inicializado");
				else
					puts("NO Inicializado");
				break;

			case 3: //finalizar

				puts("FINALIZAR");
				creoHeader(PCB,header,3,pagina); //PCB HEADER TIPOEJECUCION PAGINA
				//printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
				send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
				recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
				usleep(configuracion.retardo); //retardo del cpu
				if(recibi)	//Controlo que haya llegado bien
					puts("Finalizado");
				else
					puts("Error");
				printf("Numero de instrucciones ejecutadas: %d\n",PCB->numInstrucciones);

				//aviso al plani indicando que termino con este send:
				PCB->estado=4;
				send(socketPlanificador, &cambio, sizeof(flag), 0);
				//recv espero a que el plani me diga que sigo

				//aviso al plani indicando que termino con este semaforo:
				/*	struct sembuf semOperacion;		//estructura q contiene la operacion sobre el semaforo
					semOperacion.sem_num = numeroCpu ;	//el indice del semaforo q quiero modificar
					semOperacion.sem_op = 1;	//la cant de operaciones siempre 1
					semOperacion.sem_flg = 0; //un flag siempre en0
					semop (semVCPU, &semOperacion, 1); //aplico la operacion sobre el semaforo
		     	*/
				//libero el PCB si ejecuto todas las instrucciones
				//free(PCB); //LO COMENTO XQ TIRA ERROR
				break;
			case 4: //entrada-salida
			{
				puts("ENTRADA-SALIDA");
				PCB->estado=3; //bloqueo proceso
				usleep(configuracion.retardo); //retardo del cpu
				//señal al plani avisando que cambie de estado
				send(socketPlanificador, &cambio, sizeof(flag), 0);
				send(socketPlanificador, &pagina, sizeof(int), 0); //envio el tiempo del sleep
				break;
			}
			default:
			{
				puts("default");
				break;
			}
		}
			//Disminuyo el quanto
			if(PCB->quantum>0)
			{
				PCB->quantum--;
			}
			//PASO A LA OTRA INSTRUCCION
			PCB->instructionPointer	++;


	}	//FIN WHILE

	/*if(strcmp(instrucciones[PCB->instructionPointer - 1], "finalizar"))
	{
		puts("FINALIZAR");
		creoHeader(PCB,header,3,pagina); //PCB HEADER TIPOEJECUCION PAGINA
		//printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
		send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
		recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
		if(recibi)	//Controlo que haya llegado bien
			puts("Finalizado");
		else
			puts("Error");
		printf("Numero de instrucciones ejecutadas: %d\n",PCB->numInstrucciones);
	}*/

	free(instrucciones);
	free(header);

}




void iniciarCPU(t_cpu *CPUS){

	//pthread_t id= pthread_self(); //retorna el id del hilo q lo llamo
	//unsigned int tid = process_get_thread_id(); //no borrar puede servir mas adelante
	//printf("CPU hilo ID: %d conectado\n", tid); //se puede usar "htop" en la consola para verlos
	printf("CPU NUMERO: %d conectada\n", CPUS->numeroCPU); //se puede usar "htop" en la consola para verlos
	//printf("CPU hilo ID: %lu conectado\n", pthread_self());
	int status=1;		// Estructura que manjea el status de los recieve.

	//Estructuras que manejan los datos recibidos
	t_headcpu * header = (t_headcpu*)malloc(sizeof(t_headcpu));

	while(status!=0)	//MIENTRAS NO QUIERA SALIR RECIBO INSTRUCCIONES
	{
		puts("Esperando Instrucciones...\n");
		//CPU a la espera de nuevas instrucciones



		//creo un pool para saber si este socket tiene mensajes antes de recibir
		//pollfd->fd=CPUS->socketPlani;
		//pollfd->events=POLLIN;
		//poll(pollfd, 1,3500);

		sem_wait(semProduccionMsjs); //semaforo productor-consumidor
		puts("ANTES DEL RECV");
		status = recv(CPUS->socketPlani, header, sizeof(t_headcpu),0);

		puts("DESPUES DEL RECV");
		if(status!=0)	//CONTROLA QUE NO SE PIERDA LA CONEXION

		{
			switch (header->tipo_ejecucion){	//CONTROLA EL TIPO DE INSTRUCCION

			case 0:		//INSTRUCCION PARA SALIR

				//FINALIZO CONEXIONES
				puts("FINALIZANDO PROGRAMA\n");
				status=0;	//Salgo del while
				sem_post(&semSalir);	//Semaforo que controla la finalizacion de la CPU
				break;

			case 1: 	//INSTRUCCION PARA RECIBIR MSJS
			{
				long id_pcb = shmget(header->clave_pcb, sizeof(t_pcb), 0666); //reservo espacio dentro de la seccion de memoria compartida
				t_pcb *PCB;
				PCB = shmat(id_pcb,0, 0); //creo la variable y la asocio al segmento
				if (PCB == (t_pcb *)(-1))		//capturo error del shmat
					perror("shmat pcb");

				long id_ruta = shmget(header->clave_ruta, sizeof(char*), 0666); //reservo espacio dentro de la seccion de memoria compartida
				PCB->ruta= shmat(id_ruta, 0, SHM_RDONLY); //creo la variable y la asocio al segmento
				if (PCB->ruta == (char *)(-1))		//capturo error del shmat
				    perror("shmat ruta");

				printf("PCB Recibido. PID:%d Ruta: <%s>\n",PCB->PID,PCB->ruta);
				//ejecuto
				ejecutoPCB(sockets->socketMemoria,CPUS->socketPlani,PCB);	//analiza el PCB y envia a memoria si corresponde (nuevo)

				break;
			}

			default:	//PARA ERRORES EN EL TIPO DE INSTRUCCION
			{
				puts("TIPO DE INSTRUCCION NO VALIDA\n");
				break;
			}
			}
		}
		else	//SI SE PIERDE LA CONEXION SALGO
		{
			puts("Conexion perdida!");
			sem_post(&semSalir);	//Semaforo para controlar la finalizacion de la CPU
			break;
		}

	}	//FIN DEL WHILE

	//CIERRO LOS SOCKETS Y EL HEADER
	free(header);
	close(CPUS->socketPlani);
	close(sockets->socketMemoria);

}

/*Configuraciones basicas de los Sockets
 * y los Logs para el CPU
 */
int configuroSocketsYLogs (){
	cargoArchivoConfiguracion(); //carga las configuraciones basicas
	creoLogger(1);  //recive 0 para log solo x archivo| recive 1 para log x archivo y x pantalla
	log_info(logger, "Inicio Log CPU", NULL);
	puts("Conexion con el Planificador");
	int i = 0;
	CPU = (t_cpu*)malloc(sizeof(t_cpu) * (configuracion.cantHilos));
	while(i < configuracion.cantHilos){
		CPU[i].socketPlani = crearCliente(configuracion.ipPlanificador, configuracion.puertoPlanificador); //conecta con el planificador
		if (CPU[i].socketPlani==-1){	//controlo error
			puts("No se pudo conectar con el Planificador");
			perror("SOCKET PLANIFICADOR!");
			log_error(logger,"No se pudo conectar con el Planificador");
			abort();
		}
		i++;
	}

	sockets = (t_sockets*)malloc(sizeof(t_sockets));
	puts("Conexion con la Memoria");
	sockets->socketMemoria = crearCliente(configuracion.ipMemoria, configuracion.puertoMemoria);//conecta con la memoria
	if (sockets->socketMemoria==-1){		//controlo error
			puts("No se pudo concetar con el Adm. de Memoria");
			perror("SOCKET MEMORIA!");
			log_error(logger,"No se pudo conectar con el Adm. de Memoria");
			abort();
	}

	return 1;

}



