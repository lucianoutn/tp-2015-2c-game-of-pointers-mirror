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

void ejecutoInstruccion(t_header * registro_prueba, char * mensaje,char *  memoria_real, t_list * TLB, t_list * tabla_adm, int socketCliente, int serverSocket)
{
	int flag;
	// DEPENDIENDO EL TIPO DE EJECUCION
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
				flag=leerEnCache(socketCliente,TLB);
				if(!flag)
				{
					leerEnMemReal(tabla_adm,TLB, registro_prueba, serverSocket,socketCliente, memoria_real);
				}
			}
			else
			{
				leerEnMemReal(tabla_adm, TLB,registro_prueba, serverSocket,socketCliente, memoria_real);
			}
	 		break;

	 	case 1:
			printf ("Se recibio orden de escritura\n");

			// TRAIGO LA TABLA DEL PROCESO
			t_list * tablaProceso = obtenerTablaProceso(tabla_adm, registro_prueba->PID);
			// TRAIGO LA PAGINA A ESCRIBIR ( SOLO EL NODO DE LA TABLA DEL PROCESO )
			process_pag * paginaProceso = obtenerPaginaProceso(tablaProceso, registro_prueba->pagina_proceso);

			int tamanio_mensaje = registro_prueba->tamanio_msj;

			// DECLARO UN FLAG PARA SABER SI ESTABA EN LA TLB Y SE ESCRIBIO, O SI NO ESTABA
			int okTlb = verificarTlb(TLB,tamanio_mensaje, mensaje, registro_prueba);

			// SI ESTABA EN LA TLB, YA LA FUNCION ESCRIBIO Y LISTO
			if(okTlb == 1)
			{
				// SE ESCRIBIO CORRECTAMENTE PORQUE YA ESTABA CARGADA EN TLB
				printf ("SE ESCRIBIO CORRECTAMENTE PORQUE ESTABA EN LA TLB \n");
			}
			// SI NO ESTABA EN LA TLB, AHORA ME FIJO SI ESTA EN LA TABLA DE TABLAS
			else
			{
				printf("NO ESTABA EN TLB => VEO SI ESTA EN SWAP O EN MEMORIA \n");
				// SI ESTA EN SWAP
				if (paginaProceso->direccion_fisica == NULL)
				{
					printf("LA ENCONTRE EN SWAP \n");
					/*SI NO TENGO ESPACIO PARA TRAERLA, SWAPEO LA PRIMER PAGINA CARGADA (FIFO)
					* Y ESCRIBO LA QUE RECIBO DEL SWAP AL FINAL DE LA LISTA
					*/
					if ( marcosProcesoLlenos(tablaProceso))
					{
						printf("NO TENGO LUGAR PARA GUARDARLA \n");
						int verific = swapeando(tablaProceso,tabla_adm ,TLB, mensaje, serverSocket, registro_prueba);

					}
					//SI TENGO ESPACIO PARA TRAERLA, LA TRAIGO Y LA ESCRIBO
					else
					{
						printf("TENGO ESPACIO PARA TRAERLA \n");
						// Creo el header de lectura de una pagina y se lo mando al swap
						t_header * lectura_swap = crearHeaderLectura(registro_prueba);
						/* char para guardar el contenido de la pagina que tengo que traer del swap para escribir
						 * (Entendiendo que tengo que traer el contenido y escribir a continuacion)
						 */
						char * contenido_a_escribir = malloc(miContexto.tamanioMarco);

						int * status_lectura2 = envioAlSwap(lectura_swap, serverSocket, contenido_a_escribir);
						if(status_lectura2 == 1)
							printf("SE TRAJO CORRECTAMENTE EL CONTENIDO DEL SWAP Y ES ---> %s \n", contenido_a_escribir);
						else
							printf("NO SE PUDO TRAER LA PAGINA \n");

						// TENGO QUE ASIGNARLE UNA DIRECCION PARA ESCRIBIR AHI, TRAIGO UN MARCO HUECO Y ESCRIBO
						 t_marco_hueco * marco_a_llenar = list_remove(listaFramesHuecosMemR, 0);
						memcpy ( marco_a_llenar->direccion_inicio, contenido_a_escribir, miContexto.tamanioMarco);
						printf("ESCRIBI EN EL MARCO ---> %s \n", contenido_a_escribir);
						//AGREGO EL MARCO AHORA ESCRITO, A LA LISTA DE MARCOS ESCRITOS
						list_add(listaFramesMemR, marco_a_llenar);
						printf("AGREGE MARCO LLENO, AHORA TENGO ---> %d MARCOS LLENOS Y ----> %d VACIOS \n", listaFramesMemR->elements_count, listaFramesHuecosMemR->elements_count);
						//AGREGO LA PAGINA A LA TLB (VERIFICO SI ESTA LLENA Y REEMPLAZO)
						actualizarTlb(lectura_swap->PID, marco_a_llenar->numero_marco, marco_a_llenar->direccion_inicio, TLB);
					}
				}
				// SI NO ESTA EN SWAP, ENTONCES okMem TIENE LA DIRECCION DEL MARCO PARA ESCRIBIR EL MENSAJE
				else
				{
					printf("NO ESTA EN SWAP \n");
					memcpy ( paginaProceso->direccion_fisica, mensaje, tamanio_mensaje);
					printf ("ESCRIBI EN LA PAGINA: %s \n", paginaProceso->direccion_fisica);
				}
			}

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

void iniciarProceso(t_list* tabla_adm, t_header * proceso)
{
		printf("PRINCIPIO - La tabla de admimistracion de tablas tiene %d nodos \n", tabla_adm->elements_count);
		// PRIMERO CREO LA TABLA DEL PROCESO Y LA AGREGO A LA LISTA DE LISTAS DE PROCESOS JUNTO CON EL PID
		t_list * lista_proceso = crearListaProceso();
		// AGREGO UN NODO PARA CADA PAGINA A INICIALIZAR, OBVIAMENTE APUNTANDO A NULL PORQUE NO ESTAN EN MEMORIA TODAVIA
		int x = 0;

		printf("La direccion del comienzo de la tabla del proceso es: %p \n", lista_proceso);

		printf("PRINCIPIO - La tabla del proceso tiene %d nodos \n", lista_proceso->elements_count);
		// MIENTRAS FALTEN PAGINAS PARA INICIAR //
		while (x<proceso->pagina_proceso)
		{
			list_add(lista_proceso,pag_proc_create(x, NULL));
			x++;
		}

		printf("FINAL - La tabla del proceso tiene %d nodos \n", lista_proceso->elements_count);

		list_add(tabla_adm,tabla_adm_create(proceso->PID, lista_proceso) );
		printf("FINAL - La tabla de admimistracion de tablas tiene %d nodos \n", tabla_adm->elements_count);
}

int leerEnCache(int socketCliente, t_list * TLB)
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

int leerEnMemReal(t_list * tabla_adm, t_list * TLB, t_header * package, int serverSocket, int socketCliente, char * memoria_real)
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
	return 0;
}

t_list * obtenerTablaProceso(t_list * tabla_adm, int pid)
{
	bool _numeroDePid (void * p)
	{
		return(*(int *)p == pid);
	}

	int flag;
	t_tabla_adm * reg_tabla_tablas = list_find(tabla_adm, elNodoTienePidIgualA);

	// SI ENCONTRO UN REGISTRO CON ESE PID
	if (reg_tabla_tablas != NULL)
	{
		printf("Encontro la tabla, el pid es %d, la direccion %p \n", reg_tabla_tablas->pid, reg_tabla_tablas->direc_tabla_proc);
		int c =(reg_tabla_tablas->direc_tabla_proc)->elements_count;
		printf ("cantidad de elementos: %d \n", c);
		// TRAIGO LA TABLA DEL PROCESO Y LA DEVUELVO
		return reg_tabla_tablas->direc_tabla_proc;
	}
/*
		// SI LA DIRECCION = NULL ES PORQUE ESTA EN SWAP, SINO YA LA ENCONTRE EN MEMORIA
		if (pagina_proc->direccion_fisica == NULL)
		{
			// SWAPEAR PAGINA CON SWAP PARA PODER ESCRIBIR
			printf("Encontro la pagina en swap \n");
			//intercambioSwap();

			return NULL;

			/*
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
			/*
			memcpy ( pagina_proc->direccion_fisica, "Hola Man", 8);
			printf ("Escribi en el puntero: %s \n", pagina_proc->direccion_fisica);
			return 1;

			printf ("Encontro la pagina en memoria \n");
			return pagina_proc->direccion_fisica;
		}
	}
*/
	return NULL;
}

 process_pag * obtenerPaginaProceso(t_list * tabla_proceso, int pagina)
 {
	bool _numeroDePagina (void * p)
	{
		return(*(int*)p == pagina);
	}
 	 // TRAIGO LA PAGINA BUSCADA
	process_pag * pagina_proc = list_find(tabla_proceso, (void *)_numeroDePagina);
	return pagina_proc;
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
	t_tabla_adm * registro_tabla_proc = list_find(tabla_adm, elNodoTienePidIgualA);

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

int verificarTlb (t_list * TLB, int tamanio_msg, char * message, t_header * pagina)
{
	bool _numeroDePid (void * p)
	{
		return(*(int *)p == pagina->PID);
	}

	bool _numeroDePagina (void * p)
	{
		return(*(int*)p == pagina->pagina_proceso);
	}

	/* SOLO PARA PROBAR QUE ANDE LA VERIFICACION DE LA TLB
	* char * p = malloc(sizeof(char));
	* t_tlb * entrada_tlb = reg_tlb_create(0, 3, p);
	* list_add(TLB, entrada_tlb);
	*/

	t_list * subListaPid = list_filter(TLB,(void *)_numeroDePid);
	// SI NO ENCONTRO NINGUNA ENTRADA CON ESE PID
	if (subListaPid->elements_count == 0)
	{
		printf (" NO HAY NINGUNA PAGINA CON ESE PID CARGADA EN TLB \n");
		return 0;
	}
	//SI ENCONTRO ALGUNA ENTRADA EN LA TLB CON ESE PID
	else
	{
		printf (" HAY ALGUNA/S PAGINA/S CON ESE PID EN LA TLB \n");
		// VERIFICO QUE ALGUNA DE LAS ENTRADAS TENGA LA PAGINA QUE BUSCO
		t_tlb * registro_tlb = list_filter(TLB, (void *)_numeroDePagina);
		// SI LA ENCONTRO LA ESCRIBO
		if (registro_tlb != NULL)
		{
			printf ("ENCONTRE LA PAGINA EN LA SUBLISTA \n");
			memcpy (registro_tlb->direccion_fisica, message, tamanio_msg + 1);
			printf( "ESCRIBI : %s EN LA MEMORIA \n", registro_tlb->direccion_fisica);
			return 1;
		// SI NO LA ENCONTRO RETORNO 0
		}else
		{
			printf ("PARECE NO HABER NINGUNA CON ESA PAGINA \n");
			return 0;
		}

	}
}

int swapeando(t_list* tablaProceso,t_list* tabla_adm , t_list * TLB, char * mensaje, char * serverSocket, t_header * header)
{
	// TRAIGO LA PRIMER PAGINA DE LA TABLA, LA ELIMINO DE LA MISMA Y SE LA ENVIO AL SWAP
	process_pag * paginaASwapear = list_remove(tablaProceso, 0);
	printf("HIZO EL REMOVE DE pagina->%d \n", paginaASwapear->pag);

	t_header * header_escritura = crearHeaderEscritura( header->PID, paginaASwapear->pag, sizeof(paginaASwapear->direccion_fisica));
	printf("CREO EL HEADER ESCRITURA: pid->%d, ejecucion->%d \n", header_escritura->PID, header_escritura->type_ejecution);

	int * status_escritura = envioAlSwap(header_escritura, serverSocket, NULL);

	if (status_escritura == 1)
		printf("ESCRITURA - SE LO ENVIO AL SWAP CORRECTAMENTE \n");
	else
		printf ("ESCRITURA - NO SE LO ENVIO CORRECTAMENTE");

	// LE PIDO LA PAGINA QUE QUIERO ESCRIBIR, LA AGREGO AL FINAL DE LA LISTA Y LA ESCRIBO
	t_header * header_lectura = crearHeaderLectura(header);
	printf("CREO EL HEADER LECTURA, pid->%d, ejecucion->%d \n", header_lectura->PID, header_lectura->type_ejecution);

	char * contenido = malloc(miContexto.tamanioMarco);
	int * status_lectura = envioAlSwap(header_lectura, serverSocket, contenido);

	if (*status_lectura == 1)
		printf ("LECTURA - RECIBI FLAG TODO OK \n");
	else
		printf ("LECTURA - RECIBI FLAG TODO MAL \n;");
	/*
	// CONTATENO LO QUE YA TENIA + LO QUE TENGO QUE ESCRIBIR
	strcpy(paginaASwapear->direccion_fisica, contenido);
	strcat(paginaASwapear->direccion_fisica, mensaje );
	*/
	//process_pag * paginaASwapear
	//ENVIO AL SWAP LA PAGINA A ESCRIBIR
	//int status = send(serverSocket, registro_prueba, sizeof(t_header), 0);
	//swapear
}

int marcosProcesoLlenos(t_list * lista_proceso)
{
	printf("ENTRO A VERIFICAR LOS MARCOS DISPONIBLES \n");
	int x = 0;
	int paginas_ocupadas = 0;
	int paginas_disponibles = 0;
	int cantidad_paginas = lista_proceso->elements_count;
	printf ("CANTIDAD DE PAGINAS: %d \n", cantidad_paginas);
	while ( paginas_ocupadas+paginas_disponibles < cantidad_paginas )
	{
		process_pag * reg = list_get(lista_proceso, x);
		if ( reg->direccion_fisica != NULL)
			paginas_ocupadas++;
		else
			paginas_disponibles++;

		x++;

	}
	if (paginas_ocupadas == miContexto.maxMarcos)
	{
		printf("ENTRO A RETORNAR 1 \n");
		return 1;
	}
	else
	{
		printf("ENTRO A RETORNAR 0 \n");
		return 0;

	}
}

void actualizarTlb (int pid, int marco, char * direccion_memoria, t_list * TLB)
{
	// pid
	// marco
	// direccion de memoria
	if (miContexto.entradasTlb != TLB->elements_count)
	{
		printf("ANTES - CANTIDAD DE ELEMENTOS TLB---> %d \n", TLB->elements_count);
		list_add(TLB, reg_tlb_create(pid, marco, direccion_memoria));
		printf("DESPUES - CANTIDAD DE ELEMENTOS TLB---> %d \n", TLB->elements_count);
	}else
	{
		list_remove_and_destroy_element(TLB, 0, free); // ESTA BIEN USADO EL FREE AHI?
		printf("DESPUES DEL REMOVE - CANTIDAD DE ELEMENTOS TLB -----> %d \n", TLB->elements_count);
		list_add(TLB, reg_tlb_create(pid, marco, direccion_memoria));
	}

}

t_header* crearHeaderLectura(t_header * package)
{
	t_header * package_lectura = package_create(0, package->PID, package->pagina_proceso, package->tamanio_msj);
	return package_lectura;
}

t_header * crearHeaderEscritura(int pid, int pagina, int tamanio)
{
	t_header * header_escritura = package_create(1, pid, pagina, tamanio);
	return header_escritura;
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


