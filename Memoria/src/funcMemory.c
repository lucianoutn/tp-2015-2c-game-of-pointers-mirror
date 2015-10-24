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
 //printf ("Memoria reservada \n");
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
	 		log_info(logger, "Solicitud de lectura recibida del PID: %d y pagina: %d", registro_prueba->PID, registro_prueba->pagina_proceso);
			/* CUANDO SE RECIBE UNA INSTRUCCION DE LECTURA, PRIMERO SE VERIFICA LA TLB A VER SI YA FUE CARGADA
			 * RECIENTEMENTE, EN CASO CONTRARIO SE ENVIA AL SWAP, ESTE ME VA A DEVOLVER LA PAGINA A LEER,
			 * LA MEMORIA LA ALMACENA EN UN MARCO DISPONIBLE PARA EL PROCESO DE LA PAGINA Y ACTUALIZA SUS TABLAS
			 */
			//numero_pagina = registro_prueba->pagina_proceso;
			//numero_de_pid = registro_prueba->PID;

			// PRIMERO VERIFICO QUE LA TLB ESTE HABILITADA*/
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
			{
				flag=leerDesdeTlb(socketCliente,TLB, registro_prueba->PID, registro_prueba->pagina_proceso);
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
			//printf ("*********************Se recibio orden de escritura*********************\n");
			log_info(logger, "Solicitud de escritura recibida del PID: %d y pagina: %d", registro_prueba->PID, registro_prueba->pagina_proceso);

			int tamanio_mensaje = registro_prueba->tamanio_msj;

			// DECLARO UN FLAG PARA SABER SI ESTABA EN LA TLB Y SE ESCRIBIO, O SI NO ESTABA
			int okTlb = verificarTlb(TLB,tamanio_mensaje, mensaje, registro_prueba);

			// SI ESTABA EN LA TLB, YA LA FUNCION ESCRIBIO Y LISTO
			if(okTlb == 1)
			{// TRAIGO LA TABLA DEL PROCESO
				t_list * tablaProceso = obtenerTablaProceso(tabla_adm, registro_prueba->PID);

				// TRAIGO LA PAGINA A ESCRIBIR ( SOLO EL NODO DE LA TABLA DEL PROCESO )
				process_pag * paginaProceso = obtenerPaginaProceso(tablaProceso, registro_prueba->pagina_proceso);

				// SE ESCRIBIO CORRECTAMENTE PORQUE YA ESTABA CARGADA EN TLB
				actualizarTlb(registro_prueba->PID, registro_prueba->pagina_proceso, paginaProceso->direccion_fisica, TLB);
				printf ("SE ESCRIBIO CORRECTAMENTE PORQUE ESTABA EN LA TLB \n");
			}
			// SI NO ESTABA EN LA TLB, AHORA ME FIJO SI ESTA EN LA TABLA DE TABLAS
			else
			{
				// TRAIGO LA TABLA DEL PROCESO
				t_list * tablaProceso = obtenerTablaProceso(tabla_adm, registro_prueba->PID);

				// TRAIGO LA PAGINA A ESCRIBIR ( SOLO EL NODO DE LA TABLA DEL PROCESO )
				process_pag * paginaProceso = obtenerPaginaProceso(tablaProceso, registro_prueba->pagina_proceso);

				// SI ESTA EN SWAP
				if (paginaProceso->direccion_fisica == NULL)
				{
					/*SI NO TENGO ESPACIO PARA TRAERLA (TODOS LOS MARCOS DISPONIBLES PARA ESE
					 * PROCESO YA ESTAN LLENOS), SWAPEO LA PRIMER PAGINA CARGADA (FIFO)
					* Y ESCRIBO LA QUE RECIBO DEL SWAP AL FINAL DE LA LISTA
					*/
					if ( marcosProcesoLlenos(tablaProceso))
					{
						//printf("NO TENGO LUGAR PARA GUARDARLA, TENGO QUE SWAPEAR \n");
						int verific = swapeando(tablaProceso,tabla_adm ,TLB, mensaje, serverSocket, registro_prueba);

					}
					//SI TENGO ESPACIO PARA TRAERLA (CANT MAX DE MARCOS PARA ESE PROCESO
					// NO FUE ALCANZADA TODAVÍA), SI ME QUEDA MEMORIA (MARCOS) LA TRAIGO Y LA ESCRIBO
					else
					{
						// Creo el header de lectura de una pagina y se lo mando al swap
						t_header * lectura_swap = crearHeaderLectura(registro_prueba);
						/* char para guardar el contenido de la pagina que tengo que traer del swap para escribir
						 * (Entendiendo que tengo que traer el contenido y escribir a continuacion)
						 */
						char * contenido_a_escribir = malloc(miContexto.tamanioMarco);

						int status_lectura2 = envioAlSwap(lectura_swap, serverSocket, contenido_a_escribir);

						// TENGO QUE ASIGNARLE UNA DIRECCION PARA ESCRIBIR AHI, TRAIGO UN MARCO HUECO Y ESCRIBO
						 t_marco_hueco * marco_a_llenar = list_remove(listaFramesHuecosMemR, 0);
						marco_a_llenar->direccion_inicio=malloc(miContexto.tamanioMarco);
						log_info(logger, "Traje la pagina del swap, voy a escribir el marco %d", marco_a_llenar->numero_marco);
						sleep(miContexto.retardoMemoria);
						memcpy ( marco_a_llenar->direccion_inicio, contenido_a_escribir, sizeof(contenido_a_escribir));

						//AGREGO EL MARCO AHORA ESCRITO, A LA LISTA DE MARCOS ESCRITOS
						list_add(listaFramesMemR, marco_a_llenar);

						//AGREGO LA PAGINA A LA TLB (VERIFICO SI ESTA LLENA Y REEMPLAZO)
						actualizarTlb(lectura_swap->PID, lectura_swap->pagina_proceso, marco_a_llenar->direccion_inicio, TLB);

						// ACTUALIZO LA TABLA DEL PROCESO CON LA DRIECCION FISICA
						actualizarTablaProceso(tablaProceso, registro_prueba->pagina_proceso, marco_a_llenar->direccion_inicio);
					}
				}
				// SI NO ESTA EN SWAP, ENTONCES okMem TIENE LA DIRECCION DEL MARCO PARA ESCRIBIR EL MENSAJE
				else
				{
					log_info(logger, "Encontre la pagina en memoria, la escribo");
					sleep(miContexto.retardoMemoria);
					memcpy ( paginaProceso->direccion_fisica, mensaje, tamanio_mensaje);
				}
			}

	 		break;
	 	case 2:
	 		//printf("*********************Se recibio orden de inicializacion********************* \n");

	 		/* LA INICIALIZACION SE MANDA DIRECO AL SWAP PARA QUE RESERVE ESPACIO,
	 		   EL FLAG = 1 ME AVISA QUE RECIBIO OK */
	 		flag = envioAlSwap(registro_prueba, serverSocket, NULL);
	 		bool recibi;
	 		if(flag)
	 		{
	 			//creo todas las estructuras porque el swap ya inicializo
	 			iniciarProceso(tabla_adm, registro_prueba);
	 			log_info(logger, "Proceso mProc creado, numero de PID: %d y cantidad de paginas: %d"
	 											,registro_prueba->PID, registro_prueba->pagina_proceso);
	 			recibi = true;
	 			send(socketCliente,&recibi,sizeof(bool),0);
	 		}
	 		else
	 		{
	 			recibi = false;
	 			send(socketCliente,&recibi,sizeof(bool),0);
				log_error(logger, "Hubo un problema con la conexion/envio al swap");
	 		}

	 		break;
	 	case 3:
			printf ("*********************Se recibio orden de finalizacion de proceso :)********************* \n");
			matarProceso(registro_prueba, tabla_adm, TLB);
			int flag = envioAlSwap(registro_prueba, serverSocket, NULL );

			if(flag)
			{
				log_info(logger, "Se hizo conexion con swap, se envio proceso a matar y este fue recibido correctamente");
			}
			else
			{
				log_error(logger, "Hubo un problema con la conexion/envio al swap");
			}
	 		break;
	 	default:
			printf ("El tipo de ejecucion recibido no es valido\n");
	 		break;
	 	}
}

void iniciarProceso(t_list* tabla_adm, t_header * proceso)
{
		//printf("PRINCIPIO - La tabla de admimistracion de tablas tiene %d nodos \n", tabla_adm->elements_count);
		// PRIMERO CREO LA TABLA DEL PROCESO Y LA AGREGO A LA LISTA DE LISTAS DE PROCESOS JUNTO CON EL PID
		t_list * lista_proceso = crearListaProceso();

		// AGREGO UN NODO PARA CADA PAGINA A INICIALIZAR, OBVIAMENTE APUNTANDO A NULL PORQUE NO ESTAN EN MEMORIA TODAVIA
		int x = 0;

		//printf("La direccion del comienzo de la tabla del proceso es: %p \n", lista_proceso);

		//printf("PRINCIPIO - La tabla del proceso tiene %d nodos \n", lista_proceso->elements_count);
		// MIENTRAS FALTEN PAGINAS PARA INICIAR //
		while (x<proceso->pagina_proceso)
		{
			list_add(lista_proceso,pag_proc_create(x, NULL));
			x++;
		}

		//printf("FINAL - La tabla del proceso tiene %d nodos y su direccion sigue siendo %p \n", lista_proceso->elements_count, lista_proceso);

		list_add(tabla_adm,tabla_adm_create(proceso->PID, lista_proceso) );
		//printf("FINAL - La tabla de admimistracion de tablas tiene %d nodos \n", tabla_adm->elements_count);
}

int leerDesdeTlb(int socketCliente, t_list * TLB, int pid, int pagina)
{
	bool _numeroDePid (void * p)
	{
			return(*(int *)p == pid);
	}

	bool _numeroDePagina (void * p)
		{
				return(*(int *)p == pid);
		}

	// VERIFICO TODAS LAS ENTRADAS DE LA TLB QUE TIENE EL PID DEL PROCESO
	t_list * subListaProceso = list_filter(TLB, (void *)_numeroDePid);
	// SI ENCONTRE ALGUNA ENTRADA CON ESE PID
	if (subListaProceso != NULL)
	{
		// VERIFICO QUE ALGUNA DE LAS ENTRADAS TENGA LA PAGINA QUE BUSCO
		t_tlb * registro_tlb = list_find(subListaProceso, (void *)_numeroDePagina);

		// SI LA ENCONTRO LA LEO Y SE LA ENVIO AL CPU
		if (registro_tlb != NULL)
		{
			send(socketCliente,*(registro_tlb->direccion_fisica) ,miContexto.tamanioMarco,0);
			return 1;
		}else
		{
			// SI NO LA ENCONTRO ENTONCES LA BUSCO EN LA TABLA DE TABLAS, COMO CUANDO NO ESTA HABILITADA LA TLB
			return 0;
		}
	// SI LA TLB NO ESTA HABILITADA ENTONCES TENGO QUE VERIFICAR EN LA TABLA DE TABLAS
	//  SI YA ESTA CARGADA EN MEMORIA O SI ESTA EN SWAP
	}
	else
	{
		return 0;
	}
	return 1;
}

int leerEnMemReal(t_list * tabla_adm, t_list * TLB, t_header * package, int serverSocket, int socketCliente, char * memoria_real)
{
	bool _numeroDePid (void * p)
	{
			return(*(int *)p == package->PID);
	}


	int flag;
	t_tabla_adm * reg_tabla_tablas = list_find(tabla_adm, (void*)_numeroDePid);

	bool _numeroDePagina (void * p)
	{
		return(*(int*)p == package->pagina_proceso);
	}

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
			char * contenido = malloc(miContexto.tamanioMarco);
			flag = envioAlSwap(package, serverSocket, contenido );
			//SI TODO SALIO BIEN, EL SWAP CARGO LA PAGINA A LEER EN "CONTENIDO"
			if(flag)
			{
				log_info(logger, "Se hizo conexion con swap, se envio paquete a leer y este fue recibido correctamente");
				lectura(package, tabla_adm, memoria_real, contenido, TLB);
				// Como la transferencia con el swap fue exitosa, le envio la pagina al CPU
				send(socketCliente,contenido,miContexto.tamanioMarco,0);
				return 1;
			}
			else
			{
				log_error(logger, "Hubo un problema con la conexion/envio al swap");
				return 0;
			}
		}else // SI NO ESTA EN SWAP, YA CONOZCO LA DIRECCION DE SU MARCO //
		{
			// ACA NO SE SI SE ENVIA CON UN POINTER SI QUIERO MANDAR EL CONTENIDO DESDE ESA DIRECCION,
			 // HABRIA QUE PROBAR QUE ONDA

			send(socketCliente,pagina_proc->direccion_fisica,miContexto.tamanioMarco,0);

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
	t_tabla_adm * reg_tabla_tablas = list_find(tabla_adm, (void*)_numeroDePid);

	// SI ENCONTRO UN REGISTRO CON ESE PID
	if (reg_tabla_tablas != NULL)
	{
		//printf("Encontro la tabla, el pid es %d, la direccion %p \n", reg_tabla_tablas->pid, reg_tabla_tablas->direc_tabla_proc);
		int c =(reg_tabla_tablas->direc_tabla_proc)->elements_count;
		//printf ("cantidad de elementos: %d \n", c);
		// TRAIGO LA TABLA DEL PROCESO Y LA DEVUELVO
		return reg_tabla_tablas->direc_tabla_proc;
	}

	return NULL;
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

		//SI EL TIPO DE EJECUCION ES ESCRITURA, MANDO EL CONTENIDO
		if (header->type_ejecution == 1)
		{
			send(serverSocket, contenido, header->tamanio_msj, 0);
		}
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
				recv(serverSocket, (void *)contenido, miContexto.tamanioMarco,0);
			}
		}

		//close(serverSocket);

	 return flag;
}

bool numeroDePaginaIgualA(int * pagina_number)
{
	return (*pagina_number == numero_de_pagina);
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

void matarProceso(t_header * proceso_entrante, t_list * tabla_adm, t_list * TLB)
{
	numero_de_pid = proceso_entrante->PID;

	bool _numeroDePid (void * p)
	{
		return(*(int *)p == numero_de_pid);
	}

	int flag;
	t_tabla_adm * registro_tabla_proc = list_find(tabla_adm, (void*)_numeroDePid);

	if (registro_tabla_proc != NULL)
	{
		printf("ENCONTRE UN PROCESO PARA MATAR\n");

		printf ("LA TABLA DE TABLAS DE PROCESOS TIENE %d ELEMENTOS ANTES DE MATAR \n", tabla_adm->elements_count);
		t_list * tabla_proceso = registro_tabla_proc->direc_tabla_proc;

		if (tabla_proceso != NULL)
		{
			printf("ENCONTRE LA TABLA DEL PROCESO A MATAR \n");
			// ELIMINO TODOS LOS ELEMENTOS DE LA TABLA Y LA TABLA
			list_destroy_and_destroy_elements(tabla_proceso, (void *)pag_destroy);
			//ELIMINO LA REFERENCIA DE ESA TABLA DE PROCESO, DESDE LA TABLA DE PROCESOS
			list_remove_by_condition(tabla_adm, (void*)_numeroDePid);
			printf ("LA TABLA DE TABLAS DE PROCESOS TIENE %d ELEMENTOS DESPUES DE MATAR \n", tabla_adm->elements_count);
			// ME FIJO SI EN LA TLB HABIA ALGUNA REFERENCIA A ALGUNA DE SUS PAGINAS Y LAS ELIMINO
			removerEntradasTlb(TLB, proceso_entrante);
		}else
		{
			printf("NO SE ENCONTRO LA TABLA DEL PROCESO \n");
		}

	}else
	{
		printf("ESTAS QUERIENDO MATAR UN PROCESO QUE NO ESTA NI INICIADO \n");
	}

}

int removerEntradasTlb(t_list * TLB, t_header * header)
{
	int pid_a_eliminar =  header->PID;

	bool _numeroDePid (void * p)
	{
		return(*(int *)p == pid_a_eliminar);
	}
	while ( list_find(TLB,(void*)_numeroDePid) != NULL )
	{
		printf("ELEMENTOS DE LA TLB ANTES DE MATAR -> %d \n", TLB->elements_count);
		t_tlb * entrada_tlb = list_remove_by_condition(TLB,(void*)_numeroDePid);
		printf("ELEMENTOS DE LA TLB DESPUES DE MATAR -> %d \n", TLB->elements_count);
	}

}

int verificarTlb (t_list * TLB, int tamanio_msg, char * message, t_header * pagina)
{
	int * parametro = malloc(sizeof(int));
	t_tlb * registro_tlb = buscarEntradaProcesoEnTlb(TLB, pagina, parametro);

	if (registro_tlb != NULL)
	{
		//PINCHA ACAAAA NO SE PORQUE!!! --> LO PROBE Y NO PINCHA, POR LAS DUDAS LO DEJO COMENTADO
		//registro_tlb->direccion_fisica=malloc(miContexto.tamanioMarco);
		log_info(logger, "TLB HIT pagina: %d", registro_tlb->pagina);
		strcpy (registro_tlb->direccion_fisica, message);
		//printf( "ESCRIBI : %s EN LA PAGINA %d PID %d Y DIRECCION %p \n", registro_tlb->direccion_fisica, registro_tlb->pagina, registro_tlb->pid, registro_tlb->direccion_fisica);
		return 1;
		// SI NO LA ENCONTRO RETORNO 0
	}else
	{
		log_info(logger, "TLB MISS");
		//printf ("NO ENCONTRE ESA PAGINA EN LA TLB\n");
		return 0;
	}
}

t_tlb * buscarEntradaProcesoEnTlb (t_list * TLB, t_header * pagina, int * posicion)
{
	printf("buscar entrada proceso");
	int x = 0;
	int tamanio_tlb = list_size(TLB);

	while ( x < tamanio_tlb)
	{
		t_tlb * reg_tlb = list_get(TLB, x);
		if (  reg_tlb->pid == pagina->PID)
		{
			if ( reg_tlb->pagina == pagina->pagina_proceso)
			{
				//printf("ENCONTRE LA PAGINA %d Y SU DIRECCION ES --> %p Y SU POSICION %d \n ", reg_tlb->pagina, reg_tlb->direccion_fisica, x);
				*posicion = x;
				return reg_tlb;
			}
		}
		x++;
	}
	return NULL;
}

int swapeando(t_list* tablaProceso,t_list* tabla_adm , t_list * TLB, char * mensaje, char * serverSocket, t_header * header)
{
	// TRAIGO LA PRIMER PAGINA QUE SE HAYA CARGADO EN MEMORIA, LA ELIMINO Y SE LA ENVIO AL SWAP
	process_pag * pagina_a_remover = primerPaginaCargada(tablaProceso);
	log_info(logger, "Acceso a swap: Voy a swapear para traer la pagina %d porque no tengo lugar para este proceso", header->pagina_proceso);
	sleep(miContexto.retardoMemoria);

	int num_pag_to_remove = pagina_a_remover->pag;

	bool _numeroDePagina (void * p)
	{
		return(*(int*)p == num_pag_to_remove);
	}

	process_pag * paginaASwapear = list_remove_by_condition(tablaProceso, (void*)_numeroDePagina);

	t_header * header_escritura = crearHeaderEscritura( header->PID, paginaASwapear->pag, sizeof(paginaASwapear->direccion_fisica));

	int status_escritura = envioAlSwap(header_escritura, serverSocket, "KOLO");

	// LE PIDO LA PAGINA QUE QUIERO ESCRIBIR, LA AGREGO AL FINAL DE LA LISTA Y LA ESCRIBO
	t_header * header_lectura = crearHeaderLectura(header);

	char * contenido = malloc(miContexto.tamanioMarco);
	int status_lectura = envioAlSwap(header_lectura, serverSocket, contenido);

	// Escribo en mi pagina swpeada el contenido a escribir
	log_info(logger, "Escribo el marco de mi pagina swapeada");
	sleep(miContexto.retardoMemoria);
	strcpy(paginaASwapear->direccion_fisica, mensaje );
}

void actualizarTablaProceso(t_list * tabla_proceso, int num_pagina, char * direccion_marco)
{
	bool _numeroDePagina (void * p)
	{
		return(*(int*)p == num_pagina);
	}

	list_remove_by_condition(tabla_proceso, _numeroDePagina); // ¿ ME DEVUELVE LO QUE REMUEVE ESTA FUNCION?
	process_pag * pagina = pag_proc_create(num_pagina, direccion_marco);
	list_add(tabla_proceso,pagina);
}

process_pag * primerPaginaCargada(t_list * tablaProceso)
{
	int cantidad_paginas = list_size(tablaProceso);
	int x = 0;
	while (x < cantidad_paginas)
	{
		process_pag * pagina = list_get(tablaProceso, x);
		if (pagina->direccion_fisica != NULL)
		{
			return pagina;
		}
		x++;
	}
	return NULL;
}

int marcosProcesoLlenos(t_list * lista_proceso)
{
//	printf("ENTRO A VERIFICAR LOS MARCOS DISPONIBLES \n");
	int x = 0;
	int paginas_ocupadas = 0;
	int paginas_disponibles = 0;
	int cantidad_paginas = lista_proceso->elements_count;
//	printf ("CANTIDAD DE PAGINAS: %d \n", cantidad_paginas);
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
		//printf("LA CANTIDAD MAXIMA DE PAGINAS DE ESTE PROCESO ESTA CARGADA EN MEMORIA \n");
		return 1;
	}
	else
	{
		//printf("TODAVIA TENGO ESPACIO PARA GUARDAR UNA PAGINA \n");
		return 0;

	}
}

void actualizarTlb (int pid, int pagina, char * direccion_memoria, t_list * TLB)
{
	int posicion;
	t_tlb * entrada_tlb = buscarEntradaProcesoEnTlb(TLB, package_create(0,pid,pagina,0), &posicion);

	if( entrada_tlb != NULL )
	{
		t_tlb * removido = list_remove(TLB, posicion);
		list_add(TLB, removido);

	}
	// SI NO ESTA CARGADA EN LA TLB
	else
	{
		// SI TENGO ESPACIO EN LA TLB, AGREGO UNA ENTRADA
		if (miContexto.entradasTlb != TLB->elements_count)
		{
			list_add(TLB, reg_tlb_create(pid, pagina, direccion_memoria));
			// SI ESTA LLENA, REMUEVO EL PRIMER ELEMENTO Y AGREGO EL RECIEN USADO AL FINAL
		}else
		{
			t_tlb * prueba_tlb1 = list_get(TLB, 0);
			printf("----- pag = %d \n", prueba_tlb1->pagina);
			list_remove_and_destroy_element(TLB, 0, free); // ESTA BIEN USADO EL FREE AHI?
			t_tlb * prueba_tlb = list_get(TLB, 0);
			list_add(TLB, reg_tlb_create(pid, pagina, direccion_memoria));
		}
	}

}

t_header* crearHeaderLectura(t_header * package)
{
	t_header * package_lectura = package_create(0, package->PID, package->pagina_proceso, 0);
	return package_lectura;
}

t_header * crearHeaderEscritura(int pid, int pagina, int tamanio)
{
	t_header * header_escritura = package_create(1, pid, pagina, tamanio);
	return header_escritura;
}

//------SEÑALES QUE TIENE QUE RECIBIR LA MEMORIA-------------//
/*
 * Cuando la memoria recibe esta señal, debe limpiar la TLB.
 */
void tlbFlush(t_list * TLB)
{
	puts("Recibi SIGUSR1\n");
	if(TLB!= NULL)
	{
		puts("Recibi la TLB\n");
	}
	else
	{
		puts("No recibi nada men \n");
	}
	if (!strcmp(miContexto.tlbHabilitada,"SI"))
	{
		puts("Uy, voy a vaciar la TLB\n");
		int a=list_size(TLB);
		printf("La TLB tiene %d elementos", a);
		int tamanioTLB = list_size(TLB);
		int i=0;
		for(;i<tamanioTLB;i++)
		{
			list_remove_and_destroy_element(TLB, i, (void *)reg_tlb_destroy);
		}
	}
	else
	{
		puts("La TLB NO esta habilitada campeon.\n");
	}

	printf("La TLB tiene %d elementos", list_size(TLB));
}

/*
 * Cuando se recibe esta señal, se debe limpiar completamente la memoria principal,
 * actualizando los bits que sean necesarios en las tablas de páginas de los diferentes procesos
 */
void limpiarMemoria()
{
	puts("Recibi SIGUSR2");

}

/*
 * Cuando recibe esta señal se deberá realizar un volcado (dump) del contenido de la memoria principal,
 * en el archivo log de Administrador de Memoria, creando para tal fin un proceso nuevo.
 * Se recomienda usar fork().
 */
void dumpEnLog()
{
	puts("Recibi SIGPOLL");
}
//-----------------------------------------------------------//

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


