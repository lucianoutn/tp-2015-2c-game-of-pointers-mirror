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


//se hace directo ahora, no hace falta

/* Recibe un MSJ y arma el PCB correspondiente
 * y lo devuelve (RESERVA MEMORIA)
 */
/*t_pcb* traduceMsj(t_msjRecibido * msj){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->PID = msj->PID;
	pcb->instructionPointer = msj->instructionPointer;
	pcb->numInstrucciones = msj->numInstrucciones;
	pcb->permisos = msj->permisos;
	pcb->prioridad = msj->prioridad;
	pcb->ruta = malloc(20);
	//copia la ruta y saca el \n, para poder abrir el archivo con fopen sin error
	int x;
		for (x = 0; x < 20; x++) {
			if(msj->ruta[x]!='\n')
				pcb->ruta[x] = msj->ruta[x];
			else
				pcb->ruta[x] = '\0';
		}
	return pcb;
}*/

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
int procesaInstruccion(char* instruccion, int *pagina){

	//Inicializo variables y reservo memoria
	int i=0, j=0, valor;
	char * palabra=malloc(sizeof(char*) * 20);
	char * aux =malloc(sizeof(char*) * 4);

	//Borro las variables
	memset(palabra,'\0',20);
	memset(aux,'\0',4);

	//CONTROLO QUE NO SE TERMINE LA PALABRA
	while(instruccion[i]!= ' '){
		if (instruccion[i] == '\0')
			break;
		palabra[i]=instruccion[i];
		i++;
	}
	i++;	//incremento para ver el numero de pagina

	//CONTROLO QUE NO SE TERMINE LA INSTRUCCION
	while(instruccion[i] != '\0'){
		aux[j]=instruccion[i];
		i++; j++;
	}
	//Convierto a int la cadena
	*pagina = atoi(aux);
	valor = palabraAValor(palabra);	//Guardo el valor para liberar la memoria

	//Libero memoria
	free(palabra);
	free(aux);

	//Retorno el valor del tipo de instruccion
	return valor;

}

/* Procesa los MSJ recibidos
 * y envia el MSJ correspondiente a la memoria
 */
void ejecutoPCB(int socketMemoria, t_pcb *PCB){

	int pagina = 0;
	//reservo espacio para el header
	t_header *header = malloc(sizeof(t_header));

	//flag para controlar la respuesta a las instrucciones
	flag recibi=false;

	//reservo espacio en la memoria para guardar todas las instrucciones del archivo mCod
	printf("la ruta era: %s", PCB->ruta);
	leermCod(PCB->ruta,PCB->numInstrucciones);	//primero leo para saber el numero de instrucciones
	puts("llega");
	char **instrucciones= (char**)malloc(sizeof(char**) * (PCB->numInstrucciones));

	//guardo las intrucciones
	instrucciones = (leermCod(PCB->ruta, &PCB->numInstrucciones));

	puts("Ejecutando..."); //Control (para pruebas)

	//CONTROLO INSTRUCCION POR INSTRUCCION QUE NO QUIERA FINALIZAR
	//while(strcmp(instrucciones[PCB->instructionPointer], "finalizar"))
	while(PCB->numInstrucciones > PCB->instructionPointer)
	{
		//Switch que verifica el tipo de cada instruccion
		switch(procesaInstruccion(instrucciones[PCB->instructionPointer],&pagina))
		{

			case 0: //leer

					puts("LEER");
					creoHeader(PCB,header,0,pagina); //PCB HEADER TIPOEJECUCION PAGINA
					//printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
					send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
					recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
					if(recibi)	//Controlo que haya llegado bien
						puts("Leido");
					else
						puts("NO Leido");
					break;

			case 1: //Escribir

					//HAY QUE AGREGAR EL CAMPO PARA EL MSJ Y MANDARLO
					puts("ESCRIBIR");
					creoHeader(PCB,header,1,pagina); //PCB HEADER TIPOEJECUCION PAGINA
					//printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
					send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
					recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
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
					if(recibi)	//Controlo que haya llegado bien
						puts("Finalizado");
					else
						puts("Error");
					printf("Numero de instrucciones ejecutadas: %d\n",PCB->numInstrucciones);
					//libero el PCB si ejecuto todas las instrucciones
					free(PCB);
					break;

			default:

			puts("default");
			break;

		}
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
	free(header);

}




void iniciarCPU(t_sockets *sockets){

	pthread_t id= pthread_self(); //retorna el id del hilo q lo llamo
	printf("CPU hilo ID: %d conectado", (pthread_t)id);
	int status=1;		// Estructura que manjea el status de los recieve.

	//Estructuras que manejan los datos recibidos
	t_headcpu * header = (t_headcpu*)malloc(sizeof(t_headcpu));

	while(status!=0)	//MIENTRAS NO QUIERA SALIR RECIBO INSTRUCCIONES
	{
		puts("\n\nEsperando Instrucciones...\n\n");
		//CPU a la espera de nuevas instrucciones
		sem_wait(semProduccionMsjs); //semaforo productor-consumidor
		status = recv(sockets->socketPlanificador, header, sizeof(t_headcpu),0);
		printf("clave: %d", header->clave_pcb);

		//headcpu->tipo_ejecucion=1; //PARA PROBAR NADA MAS

		if(status!=0)	//CONTROLA QUE NO SE PIERDA LA CONEXION

		{
			switch (header->tipo_ejecucion){	//CONTROLA EL TIPO DE INSTRUCCION

			case 0:		//INSTRUCCION PARA SALIR

				//FINALIZO CONEXIONES
				puts("Recibi salir, cierro conexiones\n");
				puts("FINALIZANDO PROGRAMA\n");
				status=0;	//Salgo del while
				sem_post(&semSalir);	//Semaforo que controla la finalizacion de la CPU
				break;

			case 1: 	//INSTRUCCION PARA RECIBIR MSJS
			{
				printf("clave: %d \n", header->clave_pcb);
				int id_pcb = shmget(header->clave_pcb, sizeof(t_pcb), 0666); //reservo espacio dentro de la seccion de memoria compartida
				t_pcb *PCB;
				PCB =(t_pcb*) shmat(id_pcb,0, 0); //creo la variable y la asocio al segmento
				printf("id_pcb: %d \n", id_pcb); //imprimo el identificador de la seccion(igual que el del plani)

				int id_ruta = shmget(header->clave_ruta, sizeof(char)*20, 0666); //reservo espacio dentro de la seccion de memoria compartida
				PCB->ruta= (char*) shmat(id_ruta, 0, 0); //creo la variable y la asocio al segmento
				printf("id_ruta: %d \n", id_ruta); //imprimo el identificador de la seccion

				if (PCB == (t_pcb *)(-1))		//capturo error del shmat
				    perror("shmat");

				printf("%p\n", PCB); //imprimo la direccion de variable local (notese que es difente a la del plani)
				printf("%d\n", PCB->PID);
				printf("%d\n", PCB->instructionPointer);
				printf("%d\n", PCB->numInstrucciones);

				printf("Ruta: %s\n", PCB->ruta); //imprimo la ruta

				printf("PCB Recibido. PID:%d\n",PCB->PID);

				puts("hasta aca anda bien");

				ejecutoPCB(sockets->socketMemoria,PCB);	//analiza el PCB y envia a memoria si corresponde (nuevo)

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
	close(sockets->socketPlanificador);
	close(sockets->socketMemoria);

}

/*Configuraciones basicas de los Sockets
 * y los Logs para el CPU
 */
int configuroSocketsYLogs (t_sockets *sockets){
	cargoArchivoConfiguracion(); //carga las configuraciones basicas
	creoLogger(1);  //recive 0 para log solo x archivo| recive 1 para log x archivo y x pantalla
	log_info(logger, "Inicio Log CPU", NULL);
	puts("Conexion con el Planificador");
	sockets->socketPlanificador = crearCliente(configuracion.ipPlanificador, configuracion.puertoPlanificador); //conecta con el planificador
	if (sockets->socketPlanificador==-1){	//controlo error
			puts("No se pudo conectar con el Planificador");
			perror("SOCKET PLANIFICADOR!");
			log_error(logger,"No se pudo conectar con el Planificador");
			abort();
	}

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



