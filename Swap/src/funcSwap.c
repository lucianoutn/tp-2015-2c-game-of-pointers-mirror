/*
 * funcSwap.c
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */


#include "funcSwap.h"
#include <commons/config.h>
#include <commons/string.h>
#include <string.h>
#include <sys/mman.h>

void creoLogger()
{
    logger = log_create("log.txt", "TEST",true, LOG_LEVEL_INFO);

    /*
    log_trace(logger, "LOG A NIVEL %s", "TRACE");
    log_debug(logger, "LOG A NIVEL %s", "DEBUG");
    log_info(logger, "LOG A NIVEL %s", "INFO");
    log_warning(logger, "LOG A NIVEL %s", "WARNING");
    log_error(logger, "LOG A NIVEL %s", "ERROR");
	*/

}
FILE* crearParticion() {
	FILE *archivo;
	char * comando = malloc(200);

	strcpy(comando,"dd if=/home/utnso/git/tp-2015-2c-game-of-pointers/Swap/");
	strcat(comando, contexto->nombre);
	strcpy(comando," of=/home/utnso/git/tp-2015-2c-game-of-pointers/Swap/");
	strcat(comando, contexto->nombre);
	//strcat(comando, " bs=512");
	//strcat(comando,  string_itoa(contexto->tam_pagina));

	system(comando);


	archivo = fopen(contexto->nombre, "wb+");
	if (archivo)
		printf("Particion creada y abierta\n");
	else {
		printf("Error, no se pudo crear\n");
		return NULL;
	}

	free(comando);
	return archivo;
}

void cerrarParticion() {
	if (!fclose(archivo))
		printf("Archivo Cerrado\n");
	else {
		printf("Error: archivo NO CERRADO\n");
	}
}

void traigoContexto() {
	contexto = malloc(sizeof(t_contexto));
	t_config * config_swap = config_create("resources/config.cfg");

	if (config_swap == NULL) {
		puts("Final feliz");
		abort();
	}
	contexto->puerto = config_get_string_value(config_swap, "PUERTO_ESCUCHA");
	contexto->nombre = config_get_string_value(config_swap, "NOMBRE_SWAP");
	contexto->cant_paginas = config_get_int_value(config_swap,
			"CANTIDAD_PAGINAS");
	contexto->tam_pagina = config_get_int_value(config_swap, "TAMANIO_PAGINA");
	contexto->retardo_swap = config_get_int_value(config_swap, "RETARDO_SWAP");
	contexto->retardo_compac = config_get_int_value(config_swap,
			"RETARDO_COMPACTACION");
}

void analizoPaquete(t_header package, int socketCliente) {
	global = &package;

	switch (package.type_ejecution)
	{
	case 0:
		printf("Se recibio orden de lectura\n");
		char * contenido = malloc(contexto->tam_pagina);
		leerSwap(package, contenido);
		free(contenido);
		break;
	case 1:
		printf("Se recibio orden de escritura\n");
		escribirSwap(package, socketCliente);
		break;
	case 2:
		printf("Se recibio orden de inicializacion\n");
		inicializarProc(package);
		break;
	case 3:
		printf("Se recibio orden de finalizacion de proceso\n");
		finalizarProc(package);
		break;
	default:
		printf("El tipo de ejecucion recibido no es valido\n");
		abort();
		break;
	}
}

void leerSwap(t_header package, char * contenido)
{
	int status;
	t_pag * pag = list_find(lista_paginas, numeroDePid);

	if(pag!=NULL)
	{
		status = fseek(archivo,pag->inicio + (package.pagina_proceso * contexto->tam_pagina),SEEK_SET);
		status = fread(contenido, contexto->tam_pagina, 1, archivo);
		log_info(logger, "Se recibio orden de lectura: PID: %d Byte Inicial: %d Contenido: %s"
								,package.PID, pag->inicio,contenido);
	}
	else
	{
		printf("Final Feliz, pagina no encontrada");
		abort();
	}
}
void escribirSwap(t_header package, int socketCliente)
{
	int status;
	char * mensaje = malloc(package.tamanio_msj);
	status = recv(socketCliente, mensaje, package.tamanio_msj, 0);
	t_pag * pag = list_find(lista_paginas, numeroDePid);

	if(pag!= NULL)
	{
		status = fseek(archivo,pag->inicio + ((package.pagina_proceso) * contexto->tam_pagina),SEEK_SET);
		status = fwrite(mensaje, strlen(mensaje) + 1, 1, archivo);
	}
	else
	{
		printf("Final feliz, pagina no encontrada");
		abort();
	}

	free(mensaje);
	pag_destroy(pag);

}
void inicializarProc(t_header package) {
	t_hueco * hueco = buscarHueco(package.pagina_proceso);
	if (hueco != NULL) {
		//Inicializo
		list_add(lista_paginas,pag_create(package.PID, hueco->inicio, package.pagina_proceso));
		log_info(logger, "Se recibio orden de inicializacion: PID: %d Inicio: %d Bytes: %d"
				,package.PID, hueco->inicio,package.pagina_proceso * contexto->tam_pagina);
		//rellenarParticion(hueco->inicio, package.pagina_proceso);
		//Actualizo huecos
		hueco->inicio = hueco->inicio + (package.pagina_proceso * contexto->tam_pagina);

	} else {
		printf("No hay hueco \n Final feliz");
		abort();
	}

}

void finalizarProc(t_header package)
{
	//Actualizo lista huecos
	t_pag * pag = list_find(lista_paginas, numeroDePid);

	if(pag!= NULL)
	{
		list_add(lista_huecos, hueco_create(pag->inicio, pag->paginas));
		log_info(logger, "Se recibio orden de finalizacion: PID: %d Inicio: %d Bytes: %d"
						,package.PID, pag->inicio,pag->paginas * contexto->tam_pagina);
		//Actualizo lista paginas
		list_remove_and_destroy_by_condition(lista_paginas, numeroDePid,pag_destroy);
	}
	else
	{
		printf("Final Feliz, pagina no encontrada");
		abort();
	}

}

bool numeroDePid(int * pid) {
	return (*pid == global->PID);
}

t_hueco* buscarHueco(int tamanio) {
	int status = 0;
	int i = 0;
	while (status == 0)
	{
		t_hueco * hueco = list_get(lista_huecos, i);
		if (hueco->paginas >= tamanio)
		{
			status = 0;
			return hueco;
		}
		i++;
	}
	return NULL;
}

void rellenarParticion(int inicio, int paginas) {
	int i;
	char * var = "0";

	for (i = 0; i < (paginas * contexto->tam_pagina); i++) {
		fseek(archivo, inicio + i, SEEK_SET);
		fwrite(var, strlen(var), 1, archivo);
	}
}


