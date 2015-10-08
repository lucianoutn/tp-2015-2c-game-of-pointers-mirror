/*
 * funcMemory.c
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */

#include <SharedLibs/manejoListas.h>
#include "funcMemory.h"

void traigoContexto()
{
 // LEVANTO EL ARCHIVO CONFIG Y VERIFICO QUE LO HAYA HECHO CORRECTAMENTE /
 t_config * config_memory = config_create("resources/config.cfg");

 if( config_memory == NULL )
 {
  puts("Final feliz");
  abort();
 }

 // OBTENGO CONFIGURACION DEL CONFIG /
 miContexto.puertoServidor = config_get_string_value(config_memory, "PUERTO_ESCUCHA" );
 miContexto.puertoCliente = config_get_string_value(config_memory, "PUERTO_SWAP" );
 miContexto.maxMarcos = config_get_int_value(config_memory, "MAXIMO_MARCOS_POR_PROCESO");
 miContexto.cantidadMarcos = config_get_int_value(config_memory, "CANTIDAD_MARCOS");
 miContexto.tamanioMarco = config_get_int_value(config_memory, "TAMANIO_MARCO");
 miContexto.entradasTlb = config_get_int_value(config_memory, "ENTRADAS_TLB");
 miContexto.tlbHabilitada = config_get_string_value(config_memory, "TLB_HABILITADA");
 miContexto.retardoMemoria = config_get_int_value(config_memory, "RETARDO_MEMORIA");
 miContexto.algoritmoReemplazo = config_get_string_value(config_memory, "ALGORITMO_REEMPLAZO");
}

char * reservarMemoria(int capacidad_en_bytes)
{
 char * memoria = malloc(capacidad_en_bytes);
 printf ("Memoria reservada \n");
 return memoria;
}

void liberarMemoria(char * memoria_a_liberar)
{
 free (memoria_a_liberar);
 printf ("Memoria Liberada");
}
/*
char * crear_tlb()
{
 char * arreglo = (char *) malloc(4*3*sizeof(char*));
 int k = 0,i,j;

 for(i=0;i<4;i++) {
  for(j=0;j<3;j++) {
   arreglo[k] = i*3 + j;
   k++;
  }
 }
 printf ("La TLB se ha creado satisfactoriamente");
 return arreglo;
}
*/
void ejecutoInstruccion(t_header * registro_prueba, char * mensaje,char *  memoria_real, t_list * TLB, t_list * tabla_adm, int socketCliente, int serverSocket)
{
	int flag;
	// DEPENDIENDO EL TIPO DE EJECUCION, CASE
	switch (registro_prueba->type_ejecution)
	{
	 	case 0:
			printf ("Se recibio orden de lectura\n");
			/* CUANDO SE RECIBE UNA INSTRUCCION DE LECTURA, PRIMERO SE VERIFICA LA TLB A VER SI YA FUE CARGADA
			 * RECIENTEMENTE, EN CASO CONTRARIO SE ENVIA AL SWAP, ESTE ME VA A DEVOLVER LA PAGINA A LEER,
			 * LA MEMORIA LA ALMACENA EN UN MARCO DISPONIBLE PARA EL PROCESO DE LA PAGINA Y ACTUALIZA SUS TABLAS
			 */
			numero_pagina = registro_prueba->pagina_proceso;
			numero_de_pid = registro_prueba->PID;
			// PRIMER VERIFICO QUE LA TLB ESTE HABILITADA*/
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
			{
				flag=buscarEnCache(socketCliente,TLB);
				if(!flag)
				{
					buscarEnMemReal(tabla_adm,TLB, registro_prueba, serverSocket,socketCliente, memoria_real);
				}
			}
			else
			{
				buscarEnMemReal(tabla_adm, TLB,registro_prueba, serverSocket,socketCliente, memoria_real);
			}
	 		break;

	 	case 1:
			printf ("Se recibio orden de escritura\n");
		/*
			int PID = registro_prueba.PID;
			numero_pagina = registro_prueba.pagina_proceso;
			numero_de_pid = registro_prueba.PID;
			// VERIFICO QUE EL PID ESTE CARGADO EN LA TLB
			t_tlb * registro_tlb = list_find(TLB, elNodoTienePidIgualA);
			if (registro_tlb != NULL)
			{
				printf ("ENCONTRE EL REGISTRO CON PID %d \n ",registro_tlb->pid);
				pag_proceso * pag = list_find(registro_tlb->direccion_fisica, numeroDePaginaIgualA);
				// VERIFICO QUE LA PAGINA A ESCRIBIR ESTE CARGADA EN LA CACHE
				if (pag != NULL)
				{
					printf("LA PAGINA ESTA EN CACHE \n");
					memcpy (pag->direc_mem, mensaje, 5);
					printf( "ESCRIBI : %s EN LA MEMORIA \n", pag->direc_mem);
				}
				else
				{
					printf("SEGUI PARTICIPANDO AMIGO \n");
				}
			}
			else
			{
				printf ("NO ESTÁ EN CACHE");
				// ME FIJO SI ESTÁ EN MEMORIA REAL
			}
			//printf("ACABO DE ESCRIBIR: %s", point_to_write);
	*/
	 		break;
	 	case 2:
	 		printf("Se recibio orden de inicializacion \n");
	 		iniciarProceso(tabla_adm, registro_prueba);
	 		/* LA INICIALIZACION SE MANDA DIRECO AL SWAP PARA QUE RESERVE ESPACIO,
	 		   EL FLAG = 1 ME AVISA QUE RECIBIO OK */

	 		flag = envioAlSwap(registro_prueba, serverSocket, NULL);
	 		if(flag)
	 		{
	 			bool recibi = true;
	 			send(socketCliente,&recibi,sizeof(bool),0);
	 			log_info(logger, "Se hizo conexion con swap, se envio proceso a iniciar y este fue recibido correctamente");
	 		}
	 		else
	 		{
				log_error(logger, "Hubo un problema con la conexion/envio al swap");
	 		}

	 		break;
	 	case 3:
			printf ("Se recibio orden de finalizacion de proceso :) \n");
			matarProceso(registro_prueba, tabla_adm);
			/*
			if(flag)
			{
				log_info(logger, "Se hizo conexion con swap, se envio proceso a matar y este fue recibido correctamente");
			}
			else
			{
				log_error(logger, "Hubo un problema con la conexion/envio al swap");
			}

			numero_de_pid = registro_prueba->PID;
			t_tlb * reg_tlb = list_find(TLB, elNodoTienePidIgualA);

			printf("La TLB TIENE %d ELEMENTOS ANTES DE DESTRUIR\n", TLB->elements_count);
			// DIRECCION TABLA PROCESO A ELIMINAR
			list_destroy(reg_tlb->direccion_fisica);
		//	printf("LA DIRECCION DE LA TABLA DEL PROCESO A ELIMINAR ES %s \n", direccion_tabla_proc);

			// NO ME RECONOCE EL INPUT_DESTROY DEFINIDO EN MANEJOLISTAS
		//	list_remove_and_destroy_by_condition(TLB, elNodoTienePidIgualA, input_destroy);

			list_remove_by_condition(TLB, elNodoTienePidIgualA);
			printf("LA TLB TIENE %d ELEMENTOS DESPUES DE DESTRUIR\n", TLB->elements_count);
			//list_remove_by_condition(TLB, elNodoTienePidIgualA);
			 */
	 		break;
	 	default:
			printf ("El tipo de ejecucion recibido no es valido\n");
	 		break;
	 	}
}
/*
void iniciarEnCache(t_header registro_prueba, char * TLB, char * memoria_cache)
{
 int cant_paginas = registro_prueba.pagina_proceso;
 int x = 0, bytes = 0, a = 0;
 printf ("CANTIDAD DE PAGINAS: %d \n", cant_paginas);
 // MIENTRAS HAYA PAGINAS DEL PROCESO PARA INICIAR /
 while (x != cant_paginas)
 {
  printf("Entre %d veces + 1 \n", x);
  t_tlb * reg_input = malloc(sizeof(t_tlb));
  reg_input->pid = registro_prueba.PID;
  reg_input->pagina = x+1;
  reg_input->direccion_fisica = memoria_cache + bytes;
  bytes = bytes + miContexto.tamanioMarco;
  printf ("LA DIRECCION DE MEMORIA DEL PROCESO ES: %p \n", reg_input->direccion_fisica);
  a=list_add(TLB,reg_input);
  x++;
 }
}
*/

void iniciarProceso(t_list* tabla_adm, t_header * proceso)
{
		// PRIMERO CREO LA TABLA DEL PROCESO Y LA AGREGO A LA LISTA DE LISTAS DE PROCESOS JUNTO CON EL PID
		t_list * lista_proceso = crearListaProceso();
		// AGREGO UN NODO PARA CADA PAGINA A INICIALIZAR, OBVIAMENTE APUNTANDO A NULL PORQUE NO ESTAN EN MEMORIA TODAVIA
		int x = 0;

		// MIENTRAS FALTEN PAGINAS PARA INICIAR //
		/*
		 * KOLO!!!
		 * Para que crear esas paginas ahi, si despues la referencia a memoria la perdes
		 * no conviene ya agregarla a la lista?
		 */
		while (x<proceso->pagina_proceso)
		{
			//process_pag * pagina = pag_proc_create(x, NULL);
			list_add(lista_proceso,pag_proc_create(x, NULL));
			x++;
		}

		list_add(tabla_adm,tabla_adm_create(proceso->PID, lista_proceso) );

}

int buscarEnCache(int socketCliente, t_list * TLB)
{
	// VERIFICO TODAS LAS ENTRADAS DE LA TLB QUE TIENE EL PID DEL PROCESO
	t_list * subListaProceso = list_filter(TLB, (void *) elNodoTienePidIgualA);
	// SI ENCONTRE ALGUNA ENTRADA CON ESE PID
	if (subListaProceso != NULL)
	{
		// VERIFICO QUE ALGUNA DE LAS ENTRADAS TENGA LA PAGINA QUE BUSCO
		t_tlb * registro_tlb = list_find(subListaProceso, (void *)numeroDePaginaIgualA);

		// SI LA ENCONTRO LA LEO Y SE LA ENVIO AL CPU
		if (registro_tlb != NULL)
		{
			send(socketCliente,*(registro_tlb->direccion_fisica) ,sizeof(miContexto.tamanioMarco),0);
			return 1;
		}else
		{
			// SI NO LA ENCONTRO ENTONCES LA BUSCO EN LA TABLA DE TABLAS, COMO CUANDO NO ESTA HABILITADA LA TLB
			return 0;
		}
	/* SI LA TLB NO ESTA HABILITADA ENTONCES TENGO QUE VERIFICAR EN LA TABLA DE TABLAS
	 * SI YA ESTA CARGADA EN MEMORIA O SI ESTA EN SWAP */
	}
	else
	{
		return 0;
	}
	return 1;
}

int buscarEnMemReal(t_list * tabla_adm, t_list * TLB, t_header * package, int serverSocket, int socketCliente, char * memoria_real)
{
	int flag;
	t_tabla_adm * reg_tabla_tablas = list_find(tabla_adm, elNodoTienePidIgualA);

	// SI ENCONTRO UN REGISTRO CON ESE PID
	if (reg_tabla_tablas != NULL)
	{
		// TRAIGO LA TABLA DEL PROCESO
		t_list * tabla_proc = reg_tabla_tablas->direc_tabla_proc;

		// TRAIGO LA PAGINA BUSCADA
		process_pag * pagina_proc = list_find(tabla_proc, (void *)numeroDePaginaIgualA);

		// SI LA DIRECCION = NULL ES PORQUE ESTA EN SWAP, SINO YA LA ENCONTRE EN MEMORIA
		if (pagina_proc->direccion_fisica == NULL)
		{
			char * contenido = malloc(sizeof(miContexto.tamanioMarco));
			flag = envioAlSwap(package, serverSocket, contenido );
			//SI TODO SALIO BIEN, EL SWAP CARGO LA PAGINA A LEER EN "CONTENIDO"
			if(flag)
			{
				log_info(logger, "Se hizo conexion con swap, se envio paquete a leer y este fue recibido correctamente");
				lectura(package, tabla_adm, memoria_real, contenido, TLB);
				// Como la transferencia con el swap fue exitosa, le envio la pagina al CPU
				send(socketCliente,contenido,sizeof(miContexto.tamanioMarco),0);
				return 1;
			}
			else
			{
				log_error(logger, "Hubo un problema con la conexion/envio al swap");
				return 0;
			}
		}else // SI NO ESTA EN SWAP, YA CONOZCO LA DIRECCION DE SU MARCO //
		{
			/* ACA NO SE SI SE ENVIA CON UN POINTER SI QUIERO MANDAR EL CONTENIDO DESDE ESA DIRECCION,
			 * HABRIA QUE PROBAR QUE ONDA
			 */
			send(socketCliente,*(pagina_proc->direccion_fisica),sizeof(miContexto.tamanioMarco),0);
			return 1;
		}
	}
	return 1;
}

void lectura(t_header * proceso_entrante, t_list * tabla_adm, char * memoria_real, char * contenido, t_list * TLB)
{
	// BUSCO LA TABLA DEL PROCESO EN LA LISTA DE TABLAS DE PROCESOS POR EL NUMERO DE PID
	numero_de_pid = proceso_entrante->PID;
	t_tabla_adm * registro_tabla_proc = list_find(tabla_adm,(void*) elNodoTienePidIgualA);
	t_list * lista_proceso = registro_tabla_proc->direc_tabla_proc;

	// TRAIGO EL PRIMER MARCO VACIO DE MI MEMORIA PARA ALMACENAR EL CONTENIDO A LEER
	printf ("LA LISTA DE FRAMES HUECOS TIENE %d ELEMENTOS \n", listaFramesHuecosMemR->elements_count);
	t_marco_hueco * marco_vacio = listaFramesHuecosMemR->head;
	printf("LA DIRECCION DE MI MARCO VACIO ES %p \n", marco_vacio->direccion_inicio);

	// preguntar!! CREO LA ENTRADA DE LA PAGINA A LA TABLA DE PROCESO

	//BUSCO LA ENTRADA DE ESA PAGINA EN LISTA_PROCESO Y LA CARGO
	process_pag * pagina_proceso = list_find(lista_proceso,(void*)numeroDePaginaIgualA);
	pagina_proceso->pag = proceso_entrante->pagina_proceso;
	pagina_proceso->direccion_fisica = marco_vacio->direccion_inicio;
	/*
	 * KOLO!
	 * Aca como ya la agregue a la lista, tendria que buscarlo y rellenar con los valores correspondientes
	 *
	 * 	process_pag * pagina_proceso = malloc(sizeof(process_pag));
	  	pagina_proceso->pag = proceso_entrante->pagina_proceso;
		pagina_proceso->direccion_fisica = marco_vacio->direccion_inicio;
		list_add(lista_proceso,pagina_proceso);.
	 */
	// VERIFICO QUE HAYA AGREGADO LA PAGINA A LA TABLA
	printf("LA CANTIDAD DE PAGINAS EN MI TABLA DE PAGINAS ES %d \n", lista_proceso->elements_count);

	// PASO EL MARCO LIBRE A LA LISTA DE OCUPADOS
	t_marco * marco_ocupado = marco_create(marco_vacio->direccion_inicio, marco_vacio->numero_marco);
	list_add(listaFramesMemR, marco_ocupado);

	// VERIFICO QUE SE HAYA PASADO MI MARCO A LA LISTA DE MARCOS "OCUPADOS" PARA LLENARLO
	printf ("LA CANTIDAD DE NODOS EN MI LISTA DE FRAMES OCUPADOS ES %d \n", listaFramesMemR->elements_count);

	// ESCRIBO EN EL MARCO LA PAGINA QUE RECIBI DEL SWAP
	strcpy(pagina_proceso->direccion_fisica, contenido);

	printf("ESCRIBI EN LA PAGINA: %s", pagina_proceso->direccion_fisica);

	// LIBERO EL NODO/MARCO DE LA LISTA DE MARCOS HUECOS PORQUE AHORA ESTA EN LA DE OCUPADOS.
	list_remove(listaFramesHuecosMemR, 0);

	// SI LA TLB ESTA HABILITADA Y NO ESTA LLENA, ENTONCES LE CREO LA ENTRADA DE LA PAGINA LEIDA
	if( !strcmp(miContexto.tlbHabilitada, "SI") && !tlbLlena(TLB))
	{
	 	t_tlb * entrada_tlb = reg_tlb_create(proceso_entrante->PID, proceso_entrante->pagina_proceso, pagina_proceso->direccion_fisica);
	 	// VERIFICO LA CANTIDAD DE ELEMENTOS, A VER SI LO ESTOY HACIENDO BIEN
	 	printf("LA TLB TIENE %d ELEMENTOS \n", TLB->elements_count);
	}

}

int envioAlSwap ( t_header * header, int serverSocket, char * contenido)
{
		int flag;
		send(serverSocket, header, sizeof(t_header), 0);

		/*
		 * Una vez enviado el registro, recibo la notificación por parte del swap.
	  	 * 0 = Hubo un error.
	  	 * 1 = Todo ok.
 	 	*/
		//t_devuelvo * devuelvo = malloc(sizeof(t_devuelvo));
		//recv(serverSocket,(void*)devuelvo,sizeof(t_devuelvo),0);
		recv(serverSocket, &flag, sizeof(int),0);

		if(flag) //si no hubo error
		{
			if(header->type_ejecution==0) //si hice una lectura, devuelve la pag
			{
				recv(serverSocket, (void *)contenido, sizeof(miContexto.tamanioMarco),0);
			}
		}

		//close(serverSocket);

	 return flag;
}

bool numeroDePaginaIgualA(int * pagina_number)
{
	return (*pagina_number == numero_pagina);
}

bool elNodoTienePidIgualA(int * pid_number)
{
 return (*pid_number == numero_de_pid);

}

bool tlbLlena(t_list * TLB)
{
	int cant_elem = TLB->elements_count;
	if (cant_elem == miContexto.entradasTlb)
		return true;

	return false;
}

void matarProceso(t_header * proceso_entrante, t_list * tabla_adm)
{
	numero_de_pid = proceso_entrante->PID;
	t_tabla_adm * registro_tabla_proc = list_find(tabla_adm,(void*) elNodoTienePidIgualA);

	if (registro_tabla_proc != NULL)
	{
		printf("ENCONTRE UN PROCESO PARA MATAR\n");

		printf ("LA TABLA DE TABLAS DE PROCESOS TIENE %d ELEMENTOS ANTES DE MATAR \n", tabla_adm->elements_count);
		t_list * tabla_proceso = registro_tabla_proc->direc_tabla_proc;

		if (tabla_proceso != NULL)
		{
			printf("ENCONTRE LA TABLA DEL PROCESO A MATAR \n");
			list_destroy_and_destroy_elements(tabla_proceso, (void *)pag_destroy);
			list_remove_by_condition(tabla_adm, (void*)elNodoTienePidIgualA);

			printf ("LA TABLA DE TABLAS DE PROCESOS TIENE %d ELEMENTOS DESPUES DE MATAR \n", tabla_adm->elements_count);

		}else
		{
			printf("NO SE ENCONTRO LA TABLA DEL PROCESO \n");
		}

	}else
	{
		printf("ESTAS QUERIENDO MATAR UN PROCESO QUE NO ESTA NI INICIADO \n");
	}

}

// --------------ENTRADAS A LA TABLA DE PROCESO ------------ //
process_pag * pag_proc_create (int pagina, char * direccion_fisica)
{
 process_pag * reg_pagina = malloc(sizeof(process_pag));
 reg_pagina->pag = pagina;
 reg_pagina->direccion_fisica = direccion_fisica;
 return reg_pagina;
}

static void pag_proc_destroy(process_pag * self)
{
 free(self);
}
// --------------------------------------------------------//

