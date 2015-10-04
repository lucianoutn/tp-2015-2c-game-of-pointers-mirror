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
