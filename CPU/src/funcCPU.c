/*
 * funcCPU.c
 *
 *  Created on: 30/9/2015
 *      Author: utnso
 */


#include "funcCPU.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

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
int procesaInstruccion(char* instruccion){

	//Inicializo variables y reservo memoria
	int I=0, valor;
	char *palabra=(char*) malloc(sizeof(char));
	if (palabra == NULL) puts("ERROR MALLOC 1");

	//CONTROLO QUE NO SE TERMINE LA PALABRA
	while(instruccion[I]!= ' ')
	{
		palabra[I]=instruccion[I];
		I++;
		palabra= (char*)realloc(palabra, (I+1)*sizeof(char));
		if(instruccion[I]== ';' || instruccion[I]== '\0') break; //ignoro el ";" del final hasta llegar al"\0"
	}
	palabra[I]='\0';
	valor = palabraAValor(palabra);

	//Libero memoria
	free(palabra);

	//Retorno el valor del tipo de instruccion
	return valor;

}

char* procesaMensaje(char* instruccion){

	//Inicializo variables y reservo memoria
	int I=0, K=0;
	char *mensaje=(char*) malloc(sizeof(char));
	if (mensaje == NULL) puts("ERROR MALLOC MENSAJE");

	//SALTEO LA INSTRUCCION
	while(instruccion[I]!= ' ')
	{
		I++;
	}

	I++;

	//CONTROLO QUE NO SE TERMINE LA INSTRUCCION
	while(instruccion[I] != ' ')
	{
		I++;
	}

	I++;
	I++;

	while(instruccion[I] != '\"')
	{
		mensaje[K++]=instruccion[I++];
		mensaje = (char*)realloc(mensaje, (K)*sizeof(char));
		if (mensaje == NULL) puts("ERROR MALLOC MENSAJE");
	}

	mensaje[K] = '\0';
	//printf("EL TEXTO ES: \"%s\"\n",mensaje);

	//Retorno el valor del tipo de instruccion
	return mensaje;

}

int procesaPagina(char* instruccion){

	//Inicializo variables y reservo memoria
	int I=0, J=0, pagina;
	char *aux=(char*) malloc(sizeof(char));
	if (aux == NULL) puts("ERROR MALLOC MENSAJE");

	//SALTEO LA INSTRUCCION
	while(instruccion[I]!= ' ')
	{
		I++;
	}

	I++;

	while(instruccion[I] != ' ')
	{
		if(instruccion[I]=='\0') break;
		aux[J]=instruccion[I];
		aux= (char*)realloc(aux, (J+1)*sizeof(char));
		I++; J++;
	}

	aux[J] = '\0';
	pagina = atoi(aux);
	free(aux);
	return pagina;

}

/* Procesa los MSJ recibidos
 * y envia el MSJ correspondiente a la memoria
 */
void ejecutoPCB(int socketMemoria, int socketPlanificador, t_pcb *PCB, int *tiempoEjec){

	int pagina;
	char *mensaje;
	time_t tiempoInicio, tiempoFin; //para los time
	int tiempoEjecParcial = 0; //lo inicializo xq sino rompe el casteo de time a int
	//time_t *t1, *t2;
	//time_t *t1 = malloc(sizeof(time_t));
	//time_t *t2 = malloc(sizeof(time_t));
	resultados = queue_create();
	//reservo espacio para el header
	t_header *header = malloc(sizeof(t_header));
	if (header == NULL) puts("ERROR MALLOC 4");

	//flag para controlar la respuesta a las instrucciones por parte de la memoria
	flag recibi= false;
	//flag para avisar al planificador que cambie de estado
	flag cambio= true;

	//reservo espacio en la memoria para guardar todas las instrucciones del archivo mCod
	//primero leo para saber el numero de instrucciones
	//char **instrucciones= (char**)malloc(sizeof(char*));
	//guardo las intrucciones
	char **instrucciones = leermCod(PCB->ruta, &PCB->numInstrucciones);

	//Itera hasta tener que replanificar o finalizar
	while(PCB->estado!=3 && PCB->estado!=4 && PCB->estado!=5)
	{		//3:Bloqueado      4:Finalizado       5:Fallo
		if(PCB->quantum==0)
		{
			PCB->estado=1;
			//señal al plani avisando que cambie de estado
			send(socketPlanificador, &cambio, sizeof(flag), 0);
			break;
		}

		//Switch que verifica el tipo de cada instruccion
		switch( procesaInstruccion(instrucciones[PCB->instructionPointer]) )
		{

			case 0: //leer

				//puts("LEER");
				pagina = procesaPagina(instrucciones[PCB->instructionPointer]);
				creoHeader(PCB,header,0,pagina); //PCB HEADER TIPOEJECUCION PAGINA
				int tmno =0;
				time(&tiempoInicio);	//tomo el tiempo antes del send
				usleep(configuracion.retardo); //retardo del cpu
				send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
				recv(socketMemoria, &tmno, sizeof(int),0);		//espero recibir la respuesta
				time(&tiempoFin); //tomo el tiempo despues dl rcv
				if(tmno > 0)	//Controlo que haya llegado bien
				{
					char *contenido = (char*)malloc(sizeof(char) * tmno);
					if (contenido == NULL) puts("ERROR MALLOC 5");
					recv(socketMemoria, contenido,tmno,0);
					contenido[tmno]='\0';
					//log_info(logger, "mProc %d - Pagina %d - Leida :\"%s\"", PCB->PID, pagina, contenido);
					queue_push(resultados,resultado(0,PCB->PID,pagina,contenido,1));
					free(contenido);
				}else if(tmno == 0){
					queue_push(resultados,resultado(0,PCB->PID,pagina,NULL,0));
					//log_info(logger, "mProc %d - Pagina %d - Leida : vacia", PCB->PID, pagina);
				}else{
					queue_push(resultados,resultado(0,PCB->PID,pagina,NULL,-1));
					//log_info(logger, "mProc %d - Pagina %d - Fallo al leer",PCB->PID, pagina);
					PCB->estado =5;
					recibi=false;
					send(socketPlanificador,&recibi, sizeof(bool),0);
				}

				tiempoEjecParcial= (int)(tiempoFin - tiempoInicio); //calculo el tiempo total y lo casteo a entero
				//printf("\ntiempo ejec: %d\n", tiempoEjec); //teste

				*tiempoEjec= *tiempoEjec + tiempoEjecParcial; //para el comando CPU si se toma en cuenta metricas x cant de instrucciones
				break;

			case 1: //Escribir

				//puts("ESCRIBIR");
				pagina = procesaPagina(instrucciones[PCB->instructionPointer]);
				//printf("Mensaje recibido: <%s>\nTamaño: %d.\n",mensaje,strlen(mensaje));
				//char *mensaje;
				mensaje = procesaMensaje(instrucciones[PCB->instructionPointer]);
				creoHeader(PCB,header,1,pagina); //PCB HEADER TIPOEJECUCION PAGINA
				header->tamanio_msj = strlen(mensaje);
				time(&tiempoInicio);	//tomo el tiempo antes del send
				usleep(configuracion.retardo); //retardo del cpu
				send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
				//printf("Envio el msj: \"%s\" de: %d caracteres.\n",mensaje, header->tamanio_msj);
				send(socketMemoria, mensaje, header->tamanio_msj,0);	//envio el texto a excribir
				recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
				time(&tiempoFin); //tomo el tiempo despues dl rcv
				if(recibi){	//Controlo que haya llegado bien
					queue_push(resultados,resultado(1,PCB->PID,pagina,mensaje,1));
					//log_info(logger, "mProc %d - Pagina %d - Escrita: \"%s\"",PCB->PID, pagina, mensaje);
				}else{
					queue_push(resultados,resultado(1,PCB->PID,pagina,mensaje,-1));
					//log_info(logger, "mProc %d - Pagina %d - Fallo al escribir: %s",PCB->PID,pagina, mensaje);
					PCB->estado =5;
					send(socketPlanificador,&recibi, sizeof(flag),0);
				}
				free(mensaje);
				tiempoEjecParcial= (int)(tiempoFin - tiempoInicio); //calculo el tiempo total y lo casteo a entero
				//printf("\ntiempo ejec: %d\n", tiempoEjec); //teste
				*tiempoEjec= *tiempoEjec + tiempoEjecParcial; //para el comando CPU si se toma en cuenta metricas x cant de instrucciones
				break;

			case 2://iniciar

				//puts("INICIAR");
				pagina = procesaPagina(instrucciones[PCB->instructionPointer]);
				creoHeader(PCB,header,2,pagina); //PCB HEADER TIPOEJECUCION PAGINA
				//printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
				time(&tiempoInicio);	//tomo el tiempo antes del send
				usleep(configuracion.retardo); //retardo del cpu
				send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
				recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
				time(&tiempoFin); //tomo el tiempo despues dl rcv
				if(recibi){	//Controlo que haya llegado bien
					queue_push(resultados,resultado(2,PCB->PID,0,NULL,1));
					//log_info(logger, "mProc %d - Iniciado",PCB->PID);
					//puts("Inicializado");
				}else{
					queue_push(resultados,resultado(2,PCB->PID,0,NULL,-1));
					//log_info(logger, "mProc %d - Fallo",PCB->PID);
					//puts("NO Inicializado");
					PCB->estado =5;
					send(socketPlanificador,&recibi, sizeof(flag),0);
				}

				tiempoEjecParcial= (int)(tiempoFin - tiempoInicio); //calculo el tiempo total y lo casteo a entero
				//printf("\ntiempo ejec: %d\n", tiempoEjec); //teste
				*tiempoEjec= *tiempoEjec + tiempoEjecParcial; //para el comando CPU si se toma en cuenta metricas x cant de instrucciones
				break;

			case 3: //finalizar

				//puts("FINALIZAR");
				creoHeader(PCB,header,3,0); //PCB HEADER TIPOEJECUCION PAGINA
				time(&tiempoInicio);	//tomo el tiempo antes del retardo
				usleep(configuracion.retardo); //retardo del cpu
				time(&tiempoFin); //tomo el tiempo despues dl retrado
				send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
				recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
				if(recibi){	//Controlo que haya llegado bien
					queue_push(resultados,resultado(3,PCB->PID,0,NULL,1));
					//log_info(logger, "mProc %d - Finalizado",PCB->PID);
					PCB->estado=4;
					send(socketPlanificador, &cambio, sizeof(flag), 0);
				}else{
					queue_push(resultados,resultado(3,PCB->PID,0,NULL,-1));
					//log_info(logger, "mProc %d - Fallo al finalizar",PCB->PID);
					PCB->estado =5;
					send(socketPlanificador,&recibi, sizeof(flag),0);
					break;
				}

				tiempoEjecParcial= (int)(tiempoFin - tiempoInicio); //calculo el tiempo total y lo casteo a entero
				//printf("\ntiempo ejec: %d\n", tiempoEjec); //teste
				*tiempoEjec= *tiempoEjec + tiempoEjecParcial; //para el comando CPU si se toma en cuenta metricas x cant de instrucciones
				printf("Numero de instrucciones ejecutadas: %d\n",PCB->numInstrucciones);

				//aviso al plani indicando que termino con este semaforo:
				/*	struct sembuf semOperacion;		//estructura q contiene la operacion sobre el semaforo
					semOperacion.sem_num = numeroCpu ;	//el indice del semaforo q quiero modificar
					semOperacion.sem_op = 1;	//la cant de operaciones siempre 1
					semOperacion.sem_flg = 0; //un flag siempre en0
					semop (semVCPU, &semOperacion, 1); //aplico la operacion sobre el semaforo
		     	*/
				break;

			case 4: //entrada-salida
			{
				//puts("ENTRADA-SALIDA");
				pagina = procesaPagina(instrucciones[PCB->instructionPointer]);
				PCB->estado=3; //bloqueo proceso
				time(&tiempoInicio);	//tomo el tiempo antes del retardo
				usleep(configuracion.retardo); //retardo del cpu
				time(&tiempoFin); //tomo el tiempo despues dl retrado
				//señal al plani avisando que cambie de estado
				PCB->tiempo=pagina;
				send(socketPlanificador, &cambio, sizeof(flag), 0);
				//send(socketPlanificador, &pagina, sizeof(int), 0); //envio el tiempo del sleep
				queue_push(resultados,resultado(4,PCB->PID,pagina,NULL,1));
				//log_info(logger, "mProc %d - En entrada-salida de tiempo: %d",PCB->PID,pagina);

				tiempoEjecParcial= (int)(tiempoFin - tiempoInicio); //calculo el tiempo total y lo casteo a entero
				//printf("\ntiempo ejec: %d\n", tiempoEjec); //teste
				*tiempoEjec= *tiempoEjec + tiempoEjecParcial; //para el comando CPU si se toma en cuenta metricas x cant de instrucciones
				break;

			}
			default:
			{
				puts("default\n");
				break;
			}
		}//FIN SWITCH

		//Disminuyo el quanto
		if(PCB->quantum>0)
		{
			PCB->quantum--;
		}

		if(PCB->estado!=4 && PCB->estado!=5)
			//PASO A LA OTRA INSTRUCCION
			PCB->instructionPointer	++;

	}	//FIN WHILE

	while(!queue_is_empty(resultados))
	{
		imprimeResultados(queue_pop(resultados));
	}
	//printf("Numero de instrucciones ejecutadas en la refaga: %d\n",(PCB->instructionPointer + 1));
	int I=0;
	while(I!=PCB->numInstrucciones)
	{
		free(instrucciones[I]);
		I++;
	}

	free(instrucciones);
	free(header);
	//queue_destroy(resultados);

}

void timer(){ //funcion que resetea y calcula los valores de porcentaje de uso de los cpus cada 60s.

	int i;
	while(1){  // aca hay q preguntar lo de esta espera activa. VER setitimer()
		sleep(60); // cambiarlo a 60 segs cuando ande todo bien
		for (i=1; i <= configuracion.cantHilos; i++){
		//	printf("\ntiempoEjec: %d\n", CPU[i].tiempoEjec);	//teste
		//CPU[i].porcentajeUso = ((double)(CPU[i].tiempoEjec) * 100) / 60; //si se toma en cuenta el tiempo
		CPU[i].porcentajeUso = ((CPU[i].tiempoEjec) * 100) / 60; //si se toma en cuenta la cantidad de instrucciones
		//reseteo los valores
		CPU[i].tiempoEjec = 0;
		CPU[i].cantInstrucEjec = 0;
		}
		//setitimer(,,NULL);

	}
}


void iniciarCPU(t_cpu *CPUS){

	//pthread_t *hiloTimer;
	//pthread_create(&hiloTimer, NULL, (void*)timer, &CPUS); //este hilo es uno x cpu y va a quedar bloqueado contando
	//puts("cree el hiloTimer"); //test
	//pthread_t id= pthread_self(); //retorna el id del hilo q lo llamo
	//unsigned int tid = process_get_thread_id(); //no borrar puede servir mas adelante
	//printf("CPU hilo ID: %d conectado\n", tid); //se puede usar "htop" en la consola para verlos
	//printf("CPU: %d CONECTADA!\n", CPUS->numeroCPU); //se puede usar "htop" en la consola para verlos
	//printf("CPU hilo ID: %lu conectado\n", pthread_self());
	int status=1;		// Estructura que manjea el status de los recieve.

	//Estructuras que manejan los datos recibidos
	t_headcpu * header = (t_headcpu*)malloc(sizeof(t_headcpu));
	if (header == NULL) puts("ERROR MALLOC 6");

	while(status!=0)	//MIENTRAS NO QUIERA SALIR RECIBO INSTRUCCIONES
	{
		printf("CPU: %d Esperando Instrucciones...\n", CPUS->numeroCPU);
		//CPU a la espera de nuevas instrucciones



		//creo un pool para saber si este socket tiene mensajes antes de recibir
		//pollfd->fd=CPUS->socketPlani;
		//pollfd->events=POLLIN;
		//poll(pollfd, 1,3500);

		//sem_wait(semProduccionMsjs); //semaforo productor-consumidor
		//puts("ANTES DEL RECV");
		status = recv(CPUS->socketPlani, header, sizeof(t_headcpu),0);

		//puts("DESPUES DEL RECV");
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
				t_pcb *PCB = shmat(id_pcb,0, 0); //creo la variable y la asocio al segmento
				if (PCB == (t_pcb *)(-1))		//capturo error del shmat
					perror("shmat pcb");

				long id_ruta = shmget(header->clave_ruta, sizeof(char*), 0666); //reservo espacio dentro de la seccion de memoria compartida
				PCB->ruta= shmat(id_ruta, 0, SHM_RDONLY); //creo la variable y la asocio al segmento
				if (PCB->ruta == (char *)(-1))		//capturo error del shmat
				    perror("shmat ruta");

				//printf("PCB Recibido. PID:%d Ruta: <%s>\n",PCB->PID,PCB->ruta);
				printf("Ejecuto PCB PID:%d en la CPU: %d\n",PCB->PID,CPUS->numeroCPU);
				//ejecuto
				ejecutoPCB(CPUS->socketMem,CPUS->socketPlani,PCB, &CPUS->tiempoEjec);	//analiza el PCB y envia a memoria si corresponde (nuevo)

				break;
			}

			default:	//PARA ERRORES EN EL TIPO DE INSTRUCCION
			{
				puts("TIPO DE INSTRUCCION NO VALIDA. ERROR AL RECIBIR DEL PLANIFICADOR\n");
				break;
			}
			}
		}
		else	//SI SE PIERDE LA CONEXION SALGO
		{
			log_info(logger, "Conexion perdida", NULL);

			break;
		}

	}	//FIN DEL WHILE

	//pthread_cancel(hiloTimer);
	//CIERRO LOS SOCKETS Y EL HEADER
	free(header);
	close(CPUS->socketPlani);
	close(CPUS->socketMem);
	sem_post(&semSalir);	//Semaforo para controlar la finalizacion de la CPU

}

/*Configuraciones basicas de los Sockets
 * y los Logs para el CPU
 */
int configuroSocketsYLogs (){
	cargoArchivoConfiguracion(); //carga las configuraciones basicas
	creoLogger(1);  //recive 0 para log solo x archivo| recive 1 para log x archivo y x pantalla
	log_info(logger, "Inicio Log CPU", NULL);
	log_info(logger, "Conectado a el Planificador", NULL);
	log_info(logger, "Conectado a la Memoria", NULL);
	int i = 1;
	CPU = (t_cpu*)malloc(sizeof(t_cpu) * ((configuracion.cantHilos) + 1));
	if (CPU == NULL) puts("ERROR MALLOC 7");
	//conexion para el comandoCpu
	//afuera del while para q no se conecte con la memoria. solo c el plani
	CPU[0].porcentajeUso=0;
	CPU[0].cantInstrucEjec=0;
	CPU[0].tiempoEjec=0;
	CPU[0].socketPlani = crearCliente(configuracion.ipPlanificador, configuracion.puertoPlanificador); //conecta con el planificador
	if (CPU[0].socketPlani==-1){	//controlo error
		puts("No se pudo conectar con el Planificador");
		perror("SOCKET PLANIFICADOR!");
		log_error(logger,"No se pudo conectar con el Planificador");
		abort();
	}

	while(i <= configuracion.cantHilos){
		CPU[i].porcentajeUso=0;
		CPU[i].cantInstrucEjec=0;  //para el comando CPU si se toma en cuenta metricas x cant de instrucciones
		CPU[i].tiempoEjec=0; //testeo, volver a 0 cuando ande todo bien!
		CPU[i].socketPlani = crearCliente(configuracion.ipPlanificador, configuracion.puertoPlanificador); //conecta con el planificador
		if (CPU[i].socketPlani==-1){	//controlo error
			puts("No se pudo conectar con el Planificador");
			perror("SOCKET PLANIFICADOR!");
			log_error(logger,"No se pudo conectar con el Planificador");
			abort();
		}
		//i++;
		CPU[i].socketMem = crearCliente(configuracion.ipMemoria, configuracion.puertoMemoria);//conecta con la memoria
		if (CPU[i].socketMem==-1){		//controlo error
				puts("No se pudo concetar con el Adm. de Memoria");
				perror("SOCKET MEMORIA!");
				log_error(logger,"No se pudo conectar con el Adm. de Memoria");
				abort();
		}
		i++;
	}

	//sockets = (t_sockets*)malloc(sizeof(t_sockets));
	//log_info(logger, "Conectado a la Memoria", NULL);
	/*sockets->socketMemoria = crearCliente(configuracion.ipMemoria, configuracion.puertoMemoria);//conecta con la memoria
	if (sockets->socketMemoria==-1){		//controlo error
			puts("No se pudo concetar con el Adm. de Memoria");
			perror("SOCKET MEMORIA!");
			log_error(logger,"No se pudo conectar con el Adm. de Memoria");
			abort();
	}*/

	return 1;

}

void comandoCpu (int socket){	//Comando que devuelve el porcentaje de uso de la CPU
	int status = 1, nro;
	while (status){
		//puts("bloqueado antes del reciv"); //test
		status = recv(socket, &nro, sizeof(int),0);
		for(nro=1; nro<=configuracion.cantHilos; nro++)
		{
			send(socket, &CPU[nro].porcentajeUso, sizeof(int),0);
			//puts("hcie el envio"); //test
		}
	}//FIN WHILE
}

void imprimeResultados(t_resultados *resultado)
{
	switch(resultado->codigo)
	{
	case 0:
		if(resultado->flag == 1){
			log_info(logger, "mProc %d - Pagina %d - Leida :\"%s\"", resultado->pid, resultado->pagina, resultado->mensaje);
			free(resultado->mensaje);
		}else if(resultado->flag ==0){
			log_info(logger, "mProc %d - Pagina %d - Leida : vacia", resultado->pid, resultado->pagina);
		}else{
			log_info(logger, "mProc %d - Pagina %d - Fallo al leer",resultado->pid, resultado->pagina);
		}
		break;
	case 1:
		if(resultado->flag==1){
			log_info(logger, "mProc %d - Pagina %d - Escrita: \"%s\"",resultado->pid, resultado->pagina, resultado->mensaje);
		}else{
			log_info(logger, "mProc %d - Pagina %d - Fallo al Escribir: \"%s\"",resultado->pid, resultado->pagina, resultado->mensaje);
		}
		free(resultado->mensaje);
		break;
	case 2:
		if(resultado->flag==1){
			log_info(logger, "mProc %d - Iniciado", resultado->pid);
		}else{
			log_info(logger, "mProc %d - Fallo",resultado->pid);
		}
		break;
	case 3:
		if(resultado->flag==1){
			log_info(logger, "mProc %d - Finalizado", resultado->pid);
		}else{
			log_info(logger, "mProc %d - Fallo al finalizar",resultado->pid);
		}
		break;
	case 4:
		log_info(logger, "mProc %d - En entrada-salida de tiempo: %d",resultado->pid,resultado->pagina);
		break;

	}
	free(resultado);
}

t_resultados* resultado(int codigo, int pid, int pagina, char* mensaje,int flag)
{
	t_resultados *resultado = (t_resultados*)malloc(sizeof(t_resultados));
	if (resultado == NULL) puts("ERROR MALLOC 8");
	resultado->codigo = codigo;
	resultado->pid = pid;
	resultado->pagina = pagina;
	resultado->flag = flag;
	if(mensaje != NULL)
	{
		resultado->mensaje = (char*)malloc(strlen(mensaje)+1);
		if (resultado->mensaje == NULL) puts("ERROR MALLOC 9");
		strcpy(resultado->mensaje,mensaje);
	}else
		resultado->mensaje=mensaje;
	return resultado;
}
