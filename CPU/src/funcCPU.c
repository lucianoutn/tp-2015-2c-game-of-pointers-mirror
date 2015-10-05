/*
 * funcCPU.c
 *
 *  Created on: 30/9/2015
 *      Author: utnso
 */


#include "funcCPU.h"


void traigoContexto()
{
 // LEVANTO EL ARCHIVO CONFIG Y VERIFICO QUE LO HAYA HECHO CORRECTAMENTE /
 t_config * config_cpu = config_create("resources/config.cfg");

 if( config_cpu == NULL )
 {
  puts("Final felize");
  abort();
 }

 // OBTENGO CONFIGURACION DEL CONFIG /
 miContexto.ipPlanificador = config_get_string_value(config_cpu, "IP_PLANIFICADOR" );
 miContexto.puertoPlanificador = config_get_string_value(config_cpu, "PUERTO_PLANIFICADOR" );
 miContexto.ipMemoria = config_get_string_value(config_cpu, "IP_MEMORIA");
 miContexto.puertoMemoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
 miContexto.cantHilos = config_get_int_value(config_cpu, "CANTIDAD_HILOS");
 miContexto.retardo = config_get_int_value(config_cpu, "RETARDO");
}


void creoHeader(t_pcb * pcb, t_header* header, int ejecucion, int pagina)
{
	//LEO INSTRUCCION DEL ARCHIVO
	header->PID = pcb->PID;
	header->pagina_proceso = pagina;
	header->tamanio_msj = 0;
	header->type_ejecution = ejecucion;
	//ARMO HEADER SEGUN PROTOCOLO
}

void iniciarCPU(){

	flag recibi=false;
	int socketPlanificador = crearCliente(miContexto.ipPlanificador, miContexto.puertoPlanificador); //conecta con el planificador
	int socketMemoria = crearCliente(miContexto.ipMemoria, miContexto.puertoMemoria);//conecta con la memoria
	const char prueba = "corto.cod\n";
	pthread_t id= pthread_self(); //retorna el id del hilo q lo llamo
	printf("CPU ID: %d conectado\n", (pthread_t)id);
	int o=0;
	//Recepcion de instrucciones

	//t_pcb * PCB;
	int status=1;		// Estructura que manjea el status de los recieve.
	printf("CPU ID: %d conectada. Esperando instrucciones:\n", (pthread_t)id);

	t_header *header = malloc(sizeof(t_header));
	t_headcpu *headcpu = malloc(sizeof(t_headcpu));
	t_msjRecibido msj;

	while(status!=0)
	{
		puts("Esperando Instrucciones\n");
		status = recv(socketPlanificador, headcpu, sizeof(t_headcpu),0);
		if(status!=0)
		{
			switch (headcpu->tipo_ejecucion)
			{
			case 0:
				//FINALIZO CONEXIONES
				puts("Recibi salir, cierro conexiones");
				status=0;
				sem_post(&semSalir);
				break;
			case 1:
				status = recv(socketPlanificador, &msj, headcpu->tamanio_msj, 0);
				t_pcb * PCB = traduceMsj(&msj);//interpreta el msj (reserva memoria para el PCB)

				printf("Recibi PCB. PID:%d\n",PCB->PID);
				//pcb.PID=PID_actual
				PCB->instructionPointer=0;//inicializo el puntero de intruccion
				//reservo espacio en la memoria para guardar todas las instrucciones del archivo mCod
				leermCod(PCB->ruta,&PCB->numInstrucciones);
				char **instrucciones= (char**)malloc(sizeof(char**) * (PCB->numInstrucciones));
				//guardo las intrucciones
				instrucciones = (leermCod(PCB->ruta, &PCB->numInstrucciones));
				puts("Instrucciones leidas"); //Control (para pruebas)
				//ciclo que envia instruccion por instruccion
				while(strcmp(instrucciones[PCB->instructionPointer], "finalizar"))
				{
					//Switch que verifica el tipo de cada instruccion
					switch(compararPalabra(interpretarIntruccion(instrucciones[PCB->instructionPointer])))
					{

						case 0: //leer

								puts("LEER");
								creoHeader(PCB,header,0,o); //PCB HEADER TIPOEJECUCION PAGINA
								//printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
								send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
								recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
								o++;
								if(recibi)
									puts("Leido");
								else
									puts("NO Leido");
								break;

						case 1: //Escribir

								//HAY QUE AGREGAR EL CAMPO PARA EL MSJ Y MANDARLO
								puts("ESCRIBIR");
								creoHeader(PCB,header,1,0); //PCB HEADER TIPOEJECUCION PAGINA
								//printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
								send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
								recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
								if(recibi)
									puts("Recibi ok");
								else
								puts("Error");
								break;

						case 2://iniciar

								puts("INICIAR");
								creoHeader(PCB,header,2,2); //PCB HEADER TIPOEJECUCION PAGINA
								//printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
								send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
								recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
								if(recibi)
									puts("Inicializado");
								else
									puts("NO Inicializado");
								break;

						case 3: //finalizar

								/*puts("FINALIZAR");
								printf("Numero de instrucciones ejecutadas: %d\n",PCB->numInstrucciones);
								creoHeader(PCB,header,3,0); //PCB HEADER TIPOEJECUCION PAGINA
								printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
								send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
								recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
								if(recibi)
									puts("Recibi ok");
								else
									puts("Error");
								NOFIN = 0; //ULTIMA INSTRUCCION SALGO DEL WHILE*/
								break;

						default:

						puts("default");
						break;

					}
				PCB->instructionPointer	++;
				}
				if(strcmp(instrucciones[PCB->instructionPointer - 1], "finalizar"))
				{
					puts("FINALIZAR");
					creoHeader(PCB,header,3,0); //PCB HEADER TIPOEJECUCION PAGINA
					//printf ("HEADER TIPO EJECUCION: %d \n", header->type_ejecution); //CONTROL (no va)
					send(socketMemoria, header, sizeof(t_header), 0);	//envio la instruccion
					recv(socketMemoria, &recibi, sizeof(flag),0);		//espero recibir la respuesta
					if(recibi)
						puts("Finalizado");
					else
						puts("Error");
					printf("Numero de instrucciones ejecutadas: %d\n",PCB->numInstrucciones);
				}
				break;
			default:
			{
				puts("TIPO DE INSTRUCCION NO VALIDA def2\n");
				break;
			}
			}
		}
		else
		{
			puts("Conexion perdida!");
			sem_post(&semSalir);
			break;
		}

	}
	free(header);
	free(headcpu);
	close(socketPlanificador);
	close(socketMemoria);	// agrego el cierre del otro socket.lucho

}

t_pcb* traduceMsj(t_msjRecibido * msj){
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
}

int compararPalabra(char *palabra)
{
	int valor;
	if(strcmp(palabra,"iniciar")==0)
	{
		valor=2;
	}
	else if (strcmp(palabra,"leer")==0)
	{
		valor=0;
	}
	else if (strcmp(palabra,"escribir")==0)
	{
		valor=1;
	}
	else if (strcmp(palabra,"entrada-salida")==0)
	{
		valor=4;
	}
	else if (strcmp(palabra,"finalizar")==0)
	{
		valor=3;
	}
	else
	{
		puts("instruccion no valida");
	}
	return valor;
}

char* interpretarIntruccion(char* instruccion)
{
	int I=0;
	char *palabra=(char*)malloc(sizeof(char));
	while(instruccion[I]!=32)
	{
		palabra[I]=instruccion[I];
		I++;
		palabra=(char*)realloc(palabra, (I+1)*sizeof(char));
	}
	return palabra;
}


