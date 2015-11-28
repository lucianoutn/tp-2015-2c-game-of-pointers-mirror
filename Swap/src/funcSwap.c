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
	char * comando = malloc(200);
	strcpy(comando,"dd if=/dev/zero");
	strcat(comando," of=/home/utnso/git/tp-2015-2c-game-of-pointers/Swap/");
	strcat(comando, contexto->nombre);
	strcat(comando, " count=");
	strcat(comando,  string_itoa(contexto->cant_paginas));
	strcat(comando, " bs=");
	strcat(comando, string_itoa(contexto->tam_pagina));
	system(comando);

	archivo = fopen(contexto->nombre, "rb+");
	if (archivo)
	{
		log_info(logger,"Particion creada y abierta");
	}
	else {
		log_error(logger,"Error, no se pudo crear\n");
		return NULL;
	}

	free(comando);
	return archivo;
}

void cerrarParticion() {
	if (!fclose(archivo))
		log_info(logger,"Archivo Cerrado");
	else {
		log_error(logger,"Error, no se pudo cerrar el archivo");
	}
}

void traigoContexto() {
	contexto = malloc(sizeof(t_contexto));
	t_config * config_swap = config_create("resources/config.cfg");

	if (config_swap == NULL) {
		puts("Final feliz");
		free(config_swap->path);
		free(config_swap->properties);
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

	free(config_swap->path);
	free(config_swap->properties);
	free(config_swap);
}

void analizoPaquete(t_header * package, int socketCliente) {
	global = package;
	int status=0;
	char * contenido = malloc(contexto->tam_pagina+1);
	switch (package->type_ejecution)
	{
	case 0:
		leerSwap(package,contenido);
		if(contenido!=NULL){
			status = 1;
			usleep(contexto->retardo_swap * 1000000);
			send(socketCliente,&status,sizeof(int),0);
			send(socketCliente,contenido,contexto->tam_pagina,0);
		}
		else{
			puts("Todo mal");
			usleep(contexto->retardo_swap * 1000000);
			send(socketCliente,&status,sizeof(int),0);
		}
		break;
	case 1:
		status = escribirSwap(package, socketCliente);
		usleep(contexto->retardo_swap * 1000000);
		send(socketCliente,&status,sizeof(int),0);
		break;
	case 2:
		status = inicializarProc(package);
		usleep(contexto->retardo_swap * 1000000);
		send(socketCliente,&status,sizeof(int),0);
		break;
	case 3:
		status = finalizarProc(package);
		usleep(contexto->retardo_swap * 1000000);
		send(socketCliente,&status,sizeof(int),0);
		break;
	default:
		log_error(logger,"El tipo de ejecucion recibido no es valido\n");
		abort();
		break;
	}
	free(contenido);
}

void leerSwap(t_header * package,char * contenido)
{
	bool _numeroDePid (void * p)
	{
		return (*(int *)p == package->PID);
	}

	t_pag * pag = list_find(lista_paginas, (void *)_numeroDePid);
	if(pag!=NULL)
	{
		fseek(archivo,pag->inicio + (package->pagina_proceso * contexto->tam_pagina),SEEK_SET);
		fread(contenido, contexto->tam_pagina, 1, archivo);
		log_info(logger, "Se proceso lectura: PID: %d Pag: %d, Byte Inicial: %d Contenido: %s"
								,package->PID, package->pagina_proceso, pag->inicio+(package->pagina_proceso * contexto->tam_pagina),contenido);
		pag->leidas=pag->leidas+1;
	}
	else
	{
		contenido=NULL;
		log_error(logger, "No se encontro la pagina solicitada");
	}
}

int escribirSwap(t_header * package, int socketCliente)
{
	bool _numeroDePid (void * p)
	{
		return (*(int *)p == package->PID);
	}

	char * mensaje = malloc(contexto->tam_pagina);

	recv(socketCliente, mensaje, contexto->tam_pagina, 0);
	t_pag * pag = list_find(lista_paginas, (void *)_numeroDePid);
	//strcpy(mensaje,"Holasir");
	if(pag!= NULL)
	{
		fseek(archivo,pag->inicio + ((package->pagina_proceso) * contexto->tam_pagina),SEEK_SET);
		fwrite(mensaje, contexto->tam_pagina, 1, archivo);
		log_info(logger, "Se proceso escritura: PID: %d Pag: %d Byte Inicial: %d Contenido: %s"
										,package->PID, package->pagina_proceso, pag->inicio+(package->pagina_proceso * contexto->tam_pagina),mensaje);
		//Actualizo cant escritas
		pag->escritas= pag->escritas+1;
		//Relleno paginas

		int relleno= pag->inicio + ((package->pagina_proceso) * contexto->tam_pagina)+ strlen(mensaje)+1;
		int final_pagina= pag->inicio+((package->pagina_proceso + 1) * contexto->tam_pagina);

		for(;relleno<final_pagina;relleno++)
		{
			fseek(archivo,relleno,SEEK_SET);
			fputc('\0',archivo);
		}

	}
	else
	{
		log_error(logger, "No se encontro la pagina solicitada");
		return 0;
	}

	free(mensaje);
	return 1;

}

int inicializarProc(t_header * package) {

	t_hueco * hueco = buscarHueco(package->pagina_proceso);

	if (hueco != NULL)
	{
		//Inicializo
		list_add(lista_paginas,pag_create(package->PID, hueco->inicio, package->pagina_proceso,0,0));
		log_info(logger, "Se proceso la inicializacion: PID: %d Inicio: %d Bytes: %d"
				,package->PID, hueco->inicio,package->pagina_proceso * contexto->tam_pagina);
		rellenarParticion(hueco->inicio, package->pagina_proceso);
		//Actualizo huecos
		hueco->inicio= hueco->inicio+package->pagina_proceso*contexto->tam_pagina;
		hueco->paginas = hueco->paginas - package->pagina_proceso;
		return 1;
	}
	else
	{
		//Si no encontro hueco, es por falta de espacion, tengo que rechazar proceso
		log_error(logger, "Rechazo proceso por falta de espacio al proceso de PID: ", package->PID);
		return 0;
	}

}

int finalizarProc(t_header* package)
{
	bool _numeroDePid (void * p)
	{
		return (*(int *)p == package->PID);
	}
	//Actualizo lista huecos
	t_pag * pag = list_find(lista_paginas, (void*)_numeroDePid);

	if(pag!= NULL)
	{
		int i =0;
		for(;i<(pag->paginas*contexto->tam_pagina);i++)
		{
			fseek(archivo, pag->inicio+ i, SEEK_SET);
			fputc('\0',archivo);
		}
		/*
		puts("VACIEEEEEEEEEEEEEEEE EL SWAP");
		char* contenido=malloc(10);
		fseek(archivo, pag->inicio, SEEK_SET);
		fread(contenido, contexto->tam_pagina, 1, archivo);

		printf("PRUEEEEEEEEEEEEEBA contenido %s: \n", contenido);
*/
		list_add(lista_huecos, hueco_create(pag->inicio, pag->paginas));
		log_info(logger, "Se proceso la finalizacion: PID: %d Inicio: %d Bytes: %d"
						,package->PID, pag->inicio,pag->paginas * contexto->tam_pagina);
		//Mostrar paginas leidas y paginas escritas
		leidasYEscritas(lista_paginas);
		//Actualizo lista paginas
		list_remove_and_destroy_by_condition(lista_paginas, (void *)_numeroDePid, (void *)pag_destroy);
		return 1;
	}
	else
	{
		puts("Final feliz, pagina no encontrada");
		log_error(logger, "No se encontro la pagina solicitada");
		return 0;
	}

}

void compactarSwap()
{
	log_info(logger, "Se comenzo con la compactacion");

	int inicio_ant = 0,pag_ant = 0;
	int tamanio_lista= list_size(lista_paginas);
	int inicio = 0,cant_pag = 0,i = 0;

	for(; i<tamanio_lista; i++)
	{
		t_pag * pagina = list_get(lista_paginas, i);
		//calculo el nuevo inicio
		inicio = inicio_ant + pag_ant * contexto->tam_pagina;
		//guardo el nodo
		inicio_ant = inicio;
		pag_ant = pagina->paginas;

		cant_pag = cant_pag + pagina->paginas;

		//actualizo el archivo
		char * contenido=malloc(contexto->tam_pagina);
		int k=0;

		for(;k<=pagina->paginas;k++)
		{
			//leo donde estaba
			fseek(archivo,pagina->inicio+k*contexto->tam_pagina,SEEK_SET);
			fread(contenido, contexto->tam_pagina, 1, archivo);
			//escribo donde debe estar
			fseek(archivo,inicio+k*contexto->tam_pagina,SEEK_SET);
			fwrite(contenido, contexto->tam_pagina, 1, archivo);

		}

		//actualizo el inicio en el nodo.
		pagina->inicio=inicio;
		free(contenido);
	}
	//actualizo lista huecos
	int tamanio_huecos = list_size(lista_huecos);
	t_hueco * hueco = list_get(lista_huecos,0);
	hueco->inicio = inicio_ant + pag_ant * contexto->tam_pagina;
	hueco->paginas = contexto->cant_paginas - cant_pag;

	int relleno= hueco->inicio;
	int final_swap= contexto->cant_paginas*contexto->tam_pagina;
	//relleno lo que queda del archivo con \0

	for(;relleno <=final_swap;relleno++)
	{
		fseek(archivo,relleno,SEEK_SET);
		fputc('\0',archivo);
	}

	int j= 1;
	for(; j<tamanio_huecos; j++)
	{
		list_remove(lista_huecos, 1);
	}

	usleep(contexto->retardo_compac * 1000000);
	log_info(logger, "Se finalizo la compactacion");
}

t_hueco* buscarHueco(int tamanio) {
	int status = 0,i = 0;
	int tamanio_huecos = list_size(lista_huecos);

	while ((status == 0)&& (i<tamanio_huecos))
	{
		t_hueco * hueco = list_get(lista_huecos, i);
		if (hueco->paginas >= tamanio)
		{
			status = 1;
			return hueco;
		}
		i++;
	}
	//si no retorne ningun hueco es porque no hay espacio o porque hay fragmentacion externa
	int pag_ocupadas=0,j = 0, pag_libres;
	int tamanio_lista_paginas=list_size(lista_paginas);

	for(;j<tamanio_lista_paginas;j++)
	{
		t_pag * pag = list_get(lista_paginas, j);
		pag_ocupadas = pag_ocupadas + pag->paginas;
	}

	pag_libres = contexto->cant_paginas - pag_ocupadas;

	if (tamanio>pag_libres) //NO TENGO ESPACIO
	{
		log_error(logger, "No hay espacio disponible");
		return NULL;
	}
	else //HAY FRAGMENTACION EXTERNA, COMPACTO Y DEVUELVO EL PRIMER HUECO
	{
		compactarSwap();
		t_hueco * hueco = list_get(lista_huecos, 0);
		return hueco;
	}

	return NULL;
}
void leidasYEscritas(t_list * lista_paginas)
{
	int tamanio = lista_paginas->elements_count;
	int totalLeidas = 0;
	int totalEscritas = 0;
	int i=0;
	for(;i<tamanio;i++)
	{
		t_pag * pag = list_get(lista_paginas, i);
		totalLeidas = totalLeidas + pag->leidas;
		totalEscritas = totalEscritas + pag->escritas;
	}
	log_info(logger, "La cantidad de paginas leidas de este proceso fue: %d", totalLeidas);
	log_info(logger, "La cantidad de paginas escritas de este proceso fue: %d", totalEscritas);
}

void rellenarParticion(int inicio, int paginas) {
	int i;

	for (i = 0; i < (paginas * contexto->tam_pagina); i++) {
		fseek(archivo, inicio + i, SEEK_SET);
		fwrite("\0", strlen("\0"), 1, archivo);
	}
}


t_list * crearListaPaginas()
{
	t_list * lista_paginas= list_create();
	return lista_paginas;
}

t_list * crearListaHuecos(int cant)
{
	t_list * lista_huecos = list_create();
	list_add(lista_huecos, hueco_create(0,cant));
	return lista_huecos;
}
