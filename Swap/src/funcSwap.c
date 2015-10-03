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

FILE* crearParticion() {
	FILE *archivo;
	char * comando = malloc(200);

	strcpy(comando,"dd if=/home/utnso/git/tp-2015-2c-game-of-pointers/Swap/");
	strcat(comando, contexto->nombre);
	strcat(comando," of=/home/utnso/git/tp-2015-2c-game-of-pointers/Swap/aux.txt");
	strcat(comando, " bs=");
	strcat(comando,  string_itoa(contexto->tam_pagina));
	strcat(comando, " count=");
	strcat(comando,  string_itoa(contexto->cant_paginas));
	system(comando);

	strcpy(comando,"dd if=/home/utnso/git/tp-2015-2c-game-of-pointers/Swap/aux.txt");
	strcat(comando," of=/home/utnso/git/tp-2015-2c-game-of-pointers/Swap/");
	strcat(comando, contexto->nombre);
	strcat(comando, " bs=");
	strcat(comando,  string_itoa(contexto->tam_pagina));
	strcat(comando, " count=");
	strcat(comando,  string_itoa(contexto->cant_paginas));
	system(comando);

	system("rm /home/utnso/git/tp-2015-2c-game-of-pointers/Swap/aux.txt");

	archivo = fopen(contexto->nombre, "rb+");
	if (archivo)
	{
		printf("Particion creada y abierta\n");
	}
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
	free(config_swap);
}

void analizoPaquete(t_header * package, int socketCliente) {
	global = package;
	int status=0;
	switch (package->type_ejecution)
	{
	case 0:
		printf("Se recibio orden de lectura\n");
		char * contenido = malloc(contexto->tam_pagina);
		t_devuelvo devuelvo;
		leerSwap(package,contenido);
		if(contenido!=NULL){
			devuelvo.status = 1;
			devuelvo.contenido = contenido;
			send(socketCliente,&devuelvo,sizeof(t_devuelvo),0);
		}
		else{
			send(socketCliente,&status,sizeof(int),0);
		}
		break;
	case 1:
		printf("Se recibio orden de escritura\n");
		status = escribirSwap(package, socketCliente);
		send(socketCliente,&status,sizeof(int),0);
		break;
	case 2:
		printf("Se recibio orden de inicializacion\n");
		status = inicializarProc(package);
		send(socketCliente,&status,sizeof(int),0);
		break;
	case 3:
		printf("Se recibio orden de finalizacion de proceso\n");
		status = finalizarProc(package);
		send(socketCliente,&status,sizeof(int),0);
		break;
	default:
		printf("El tipo de ejecucion recibido no es valido\n");
		abort();
		break;
	}
}

void leerSwap(t_header *package,char * contenido)
{
	t_pag * pag = list_find(lista_paginas, (void *)numeroDePid);
	if(pag!=NULL)
	{
		fseek(archivo,pag->inicio + (package->pagina_proceso * contexto->tam_pagina),SEEK_SET);
		fread(contenido, contexto->tam_pagina, 1, archivo);
		log_info(logger, "Se recibio orden de lectura: PID: %d Byte Inicial: %d Contenido: %s"
								,package->PID, pag->inicio+(package->pagina_proceso * contexto->tam_pagina),contenido);
	}
	else
	{
		contenido = NULL;
		puts("Final feliz, pagina no encontrada");
		log_error(logger, "No se encontro la pagina solicitada");
	}
}
int escribirSwap(t_header * package, int socketCliente)
{
	char * mensaje = malloc(package->tamanio_msj);
	strcpy(mensaje,"hola\0");
	//status = recv(socketCliente, mensaje, package.tamanio_msj, 0);
	t_pag * pag = list_find(lista_paginas, (void *)numeroDePid);

	if(pag!= NULL)
	{
		fseek(archivo,pag->inicio + ((package->pagina_proceso) * contexto->tam_pagina),SEEK_SET);
		fwrite(mensaje, strlen(mensaje) + 1, 1, archivo);

		//Relleno pagina
		int relleno= pag->inicio + strlen(mensaje) + 1;
		int final_pagina= pag->inicio+((package->pagina_proceso + 1) * contexto->tam_pagina);

		for(;relleno<=final_pagina;relleno++)
		{
			fseek(archivo,relleno,SEEK_SET);
			fwrite("0", strlen("0") + 1, 1, archivo);
		}
	}
	else
	{
		puts("Final feliz, pagina no encontrada");
		log_error(logger, "No se encontro la pagina solicitada");
		return 0;
	}

	free(mensaje);
	pag_destroy(pag);
	return 1;

}

int inicializarProc(t_header * package) {

	t_hueco * hueco = buscarHueco(package->tamanio_msj);

	if (hueco != NULL)
	{
		//Inicializo
		list_add(lista_paginas,pag_create(package->PID, hueco->inicio, package->pagina_proceso));
		log_info(logger, "Se recibio orden de inicializacion: PID: %d Inicio: %d Bytes: %d"
				,package->PID, hueco->inicio,package->pagina_proceso * contexto->tam_pagina);
		//rellenarParticion(hueco->inicio, package.pagina_proceso);
		//Actualizo huecos
		hueco->inicio = hueco->inicio + (package->pagina_proceso * contexto->tam_pagina);
		return 1;

	} else {
		puts("Final feliz");
		log_error(logger, "No hay hueco para poder escribir");
		return 0;
	}

}

int finalizarProc(t_header* package)
{
	//Actualizo lista huecos
	t_pag * pag = list_find(lista_paginas, (void*)numeroDePid);

	if(pag!= NULL)
	{
		list_add(lista_huecos, hueco_create(pag->inicio, pag->paginas));
		log_info(logger, "Se recibio orden de finalizacion: PID: %d Inicio: %d Bytes: %d"
						,package->PID, pag->inicio,pag->paginas * contexto->tam_pagina);
		//Actualizo lista paginas
		list_remove_and_destroy_by_condition(lista_paginas, (void *)numeroDePid, (void *)pag_destroy);

		return 1;
	}
	else
	{
		puts("Final feliz, pagina no encontrada");
		log_error(logger, "No se encontro la pagina solicitada");
		return 0;
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
			status = 1;
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


