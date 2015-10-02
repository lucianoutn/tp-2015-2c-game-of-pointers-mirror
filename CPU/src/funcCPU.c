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


void creoHeader(t_pcb * pcb, t_header* header)
{
	//LEO INSTRUCCION DEL ARCHIVO

	//ARMO HEADER SEGUN PROTOCOLO
}
