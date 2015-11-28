/*
 * funcMemory.c
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */
//#include <SharedLibs/manejoListas.h>
#include "funcMemory.h"
#define SLEEP 100000

void traigoContexto()
{
 // LEVANTO EL ARCHIVO CONFIG Y VERIFICO QUE LO HAYA HECHO CORRECTAMENTE /
 t_config * config_memory = config_create("resources/config.cfg");

 if( config_memory == NULL )
 {
	 puts("Final feliz");
	 free(config_memory->path);
	 free(config_memory->properties);
	 free(config_memory);
	 abort();
 }

 // OBTENGO CONFIGURACION DEL CONFIG /
 miContexto.ipSwap = config_get_string_value(config_memory, "IP_SWAP" );
 miContexto.puertoServidor = config_get_string_value(config_memory, "PUERTO_ESCUCHA" );
 miContexto.puertoCliente = config_get_string_value(config_memory, "PUERTO_SWAP" );
 miContexto.maxMarcos = config_get_int_value(config_memory, "MAXIMO_MARCOS_POR_PROCESO");
 miContexto.cantidadMarcos = config_get_int_value(config_memory, "CANTIDAD_MARCOS");
 miContexto.tamanioMarco = config_get_int_value(config_memory, "TAMANIO_MARCO");
 miContexto.entradasTlb = config_get_int_value(config_memory, "ENTRADAS_TLB");
 miContexto.tlbHabilitada = config_get_string_value(config_memory, "TLB_HABILITADA");
 miContexto.retardoMemoria = config_get_int_value(config_memory, "RETARDO_MEMORIA");
 miContexto.algoritmoReemplazo = config_get_string_value(config_memory, "ALGORITMO_REEMPLAZO");

 free(config_memory->path);
 free(config_memory->properties);
 free(config_memory);
}

char * reservarMemoria(int cantidadMarcos, int capacidadMarco)
{
	// La creo con calloc para que me la llene de \0
	char * memoria = calloc(cantidadMarcos, capacidadMarco);
	//printf ("Memoria reservada \n");
	return memoria;
}

void liberarMemoria(char * memoria_a_liberar)
{
	free (memoria_a_liberar);
	log_info(logger,"Memoria Liberada");
}

void ejecutoInstruccion(t_header * header, char * mensaje,char *  memoria_real, t_list * TLB, t_list * tabla_adm, int socketCliente, int serverSocket, t_list* tablaAccesos)
{
	int * flag = malloc(sizeof(int));
	// DEPENDIENDO EL TIPO DE EJECUCION
	switch (header->type_ejecution)
	{
	 	case 0:
	 		log_info(logger, "Solicitud de lectura recibida del PID: %d y pagina: %d", header->PID, header->pagina_proceso);
			/* CUANDO SE RECIBE UNA INSTRUCCION DE LECTURA, PRIMERO SE VERIFICA LA TLB A VER SI YA FUE CARGADA
			 * RECIENTEMENTE, EN CASO CONTRARIO SE ENVIA AL SWAP, ESTE ME VA A DEVOLVER LA PAGINA A LEER,
			 * LA MEMORIA LA ALMACENA EN UN MARCO DISPONIBLE PARA EL PROCESO DE LA PAGINA Y ACTUALIZA SUS TABLAS
			 */
			// PRIMERO VERIFICO QUE LA TLB ESTE HABILITADA*/
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
			{
				int flagg=leerDesdeTlb(socketCliente,TLB, header, tablaAccesos, tabla_adm);
				// SI NO ESTABA EN TLB, ME FIJO EN MEMORIA
				if(!flagg)
				{
					leerEnMemReal(tabla_adm,TLB, header, serverSocket,socketCliente, tablaAccesos);
				}
			}
			else
			{
				leerEnMemReal(tabla_adm, TLB,header, serverSocket,socketCliente, tablaAccesos);
			}
	 		break;
	 	case 1:
			log_info(logger, "Solicitud de escritura recibida del PID: %d y Pagina: %d, el mensaje es: \"%s\"", header->PID, header->pagina_proceso, mensaje);
			// DECLARO UN FLAG PARA SABER SI ESTABA EN LA TLB Y SE ESCRIBIO, O SI NO ESTABA
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
			{
				int okTlb = escribirDesdeTlb(TLB,header->tamanio_msj, mensaje, header, tablaAccesos, tabla_adm, socketCliente);
				// SI ESTABA EN LA TLB, YA LA FUNCION ESCRIBIO Y LISTO
				if(!okTlb)
				{
					escribirEnMemReal(tabla_adm, TLB, header,serverSocket, socketCliente,  mensaje, tablaAccesos);
				}
			}
			// SI NO ESTABA EN LA TLB, AHORA ME FIJO SI ESTA EN LA TABLA DE TABLAS
			else
			{
				escribirEnMemReal(tabla_adm, TLB, header,serverSocket, socketCliente,  mensaje, tablaAccesos);
			}
	 		break;
	 	case 2:
	 		/* LA INICIALIZACION SE MANDA DIRECO AL SWAP PARA QUE RESERVE ESPACIO,
	 		   EL FLAG = 1 ME AVISA QUE RECIBIO OK */
	 		envioAlSwap(header, serverSocket, NULL, flag);
	 		bool recibi;
	 		if(*flag == 1)
	 		{
	 			//creo todas las estructuras porque el swap ya inicializo
	 			iniciarProceso(tabla_adm, header, tablaAccesos);
	 			log_info(logger, "Proceso mProc creado, numero de PID: %d y cantidad de paginas: %d",header->PID, header->pagina_proceso);
	 			// VER COMO MANDAR LA VALIDACION AL CPU QUE NO LE ESTA LLEGANDO BIEN
	 			recibi = true;
	 			send(socketCliente,&recibi,sizeof(bool),0);
	 			log_info(logger,"Se envia al CPU confimacion de inicializacion");
	 		}
	 		else
	 		{
	 			recibi = false;
	 			send(socketCliente,&recibi,sizeof(bool),0);
				log_error(logger, "Hubo un problema con la conexion/envio al swap");
	 		}
	 		break;
	 	case 3:
			log_info(logger, "+++++++++++++++++++++Se recibio orden de finalizacion del PID: %d+++++++++++++++++++++",header->PID);
			// Lo muestro aca porque si lo muestro despues de que lo mate, la tabla no tiene el registro
			mostrarVersus(tablaAccesos, header->PID);
			matarProceso(header, tabla_adm, TLB, tablaAccesos);
			envioAlSwap(header, serverSocket, NULL, flag );

			if(flag)
			{
				log_info(logger, "Se hizo conexion con swap, se envio proceso a matar y este fue recibido correctamente");
				bool recibi = true;
				send(socketCliente,&recibi,sizeof(bool),0);
				log_info(logger,"Se informa al CPU confirmacion de finalizacion");
			}else
			{
				bool recibi = false;
				send(socketCliente,&recibi,sizeof(bool),0);
				log_error(logger, "Hubo un problema con la conexion/envio al swap");
			}
	 		break;
	 	default:
			log_error(logger, "El tipo de ejecucion recibido no es valido");
	 		break;
	 	}
}

void iniciarProceso(t_list* tabla_adm, t_header * proceso, t_list* tablaAccesos)
{
		// PRIMERO CREO LA TABLA DEL PROCESO Y LA AGREGO A LA LISTA DE LISTAS DE PROCESOS JUNTO CON EL PID
		t_list * lista_proceso = crearListaProceso();

		list_add(tablaAccesos, versus_create(proceso->PID, 0, 0));

		// AGREGO UN NODO PARA CADA PAGINA A INICIALIZAR, OBVIAMENTE APUNTANDO A NULL PORQUE NO ESTAN EN MEMORIA TODAVIA
		int x = 0;

		// MIENTRAS FALTEN PAGINAS PARA INICIAR //
		while (x<proceso->pagina_proceso)
		{
			list_add(lista_proceso,pag_proc_create(x, NULL, -1, 0 , 0, 0));
			//cuando la creo el marco lo pongo en -1
			x++;
		}

		list_add(tabla_adm,tabla_adm_create(proceso->PID, lista_proceso) );
}

int leerDesdeTlb(int socketCliente, t_list * TLB, t_header * proc, t_list* tablaAccesos, t_list* tabla_adm)
{
	bool _numeroDePid (void * p){ return(*(int *)p == proc->PID);	}
	bool _numeroDePagina (void * p)	{ return(*(int *)p == proc->pagina_proceso);}

	int * posicion = malloc(sizeof(int));
	t_tlb * registro_tlb = buscarEntradaProcesoEnTlb(TLB, proc, posicion );

	// SI LA ENCONTRO LA LEO Y LE ENVIO EL FLAG TODO JOYA AL CPU
	if (registro_tlb != NULL)
	{
		log_info(logger, "TLB HIT pagina: %d en el marco numero: %d y dice: \"%s\"", registro_tlb->pagina,registro_tlb->marco, registro_tlb->direccion_fisica);
		// SEGUN ISSUE 71, SI LA ENCUENTRA EN TLB HACE UN RETARDO SOLO, CUANDO OPERA CON LA PÁGINA (LA LEE)
		usleep(miContexto.retardoMemoria * SLEEP);

		upPaginasAccedidas(tablaAccesos, registro_tlb->pid);

		int tamanioMsj = strlen(registro_tlb->direccion_fisica)+1;

		send(socketCliente,&tamanioMsj,sizeof(int),0);
		if(tamanioMsj > 0)
			send(socketCliente, registro_tlb->direccion_fisica, tamanioMsj , 0);
		log_info(logger,"Se informa al CPU confirmacion de lectura");

		t_list * tabla_proc = obtenerTablaProceso(tabla_adm, proc->PID);
		// SI ENCONTRO UN REGISTRO CON ESE PID
		if(tabla_proc!=NULL)
		{	// TRAIGO LA PAGINA BUSCADA
			process_pag * pagina_proc= obtenerPaginaProceso(tabla_proc, proc->pagina_proceso);
			actualizoTablaProceso(tabla_proc, NULL, proc);
		}else
		{
			log_error(logger, "No se encontro la tabla del proceso");
		}
		return 1;
	}
	// SI LA TLB NO ESTA HABILITADA ENTONCES TENGO QUE VERIFICAR EN LA TABLA DE TABLAS
	//  SI YA ESTA CARGADA EN MEMORIA O SI ESTA EN SWAP
	return 0;
}

int leerEnMemReal(t_list * tabla_adm, t_list * TLB, t_header * package, int serverSocket, int socketCliente, t_list* tablaAccesos)
{
	int * flag = malloc(sizeof(int));
	usleep(miContexto.retardoMemoria * SLEEP); // SLEEP PORQUE LA MEMORIA BUSCA EN SUS ESTRUCTURAS
	t_list * tabla_proc = obtenerTablaProceso(tabla_adm, package->PID);

	// SI ENCONTRO UN REGISTRO CON ESE PID
	if(tabla_proc!=NULL)
	{	// TRAIGO LA PAGINA BUSCADA
		process_pag * pagina_proc= obtenerPaginaProceso(tabla_proc, package->pagina_proceso);
		// SI LA DIRECCION ES NULL ES PORQUE ESTA EN SWAP, SINO YA LA ENCONTRE EN MEMORIA
		if ( pagina_proc->direccion_fisica == NULL)
		{
			log_info(logger, "Se encontro la pagina para leer en swap, se hace el pedido de lectura");
			if ( marcosProcesoLlenos(tabla_proc))
			{
				int verific= swapeando(tabla_proc,tabla_adm ,TLB, NULL, serverSocket, package, tablaAccesos, socketCliente);
			}
			/* SI TENGO ESPACIO PARA TRAERLA (CANT MAX DE MARCOS PARA ESE PROCESO
			 *NO FUE ALCANZADA TODAVÍA), SI ME QUEDA MEMORIA (MARCOS) LA TRAIGO(MENTIRA)
			*/
			else
			{
				if ( listaFramesHuecosMemR->elements_count != 0)
				{
					char * contenido = malloc(miContexto.tamanioMarco);
					envioAlSwap(package, serverSocket, contenido, flag);
					//SI TODO SALIO BIEN, EL SWAP CARGO LA PAGINA A LEER EN "CONTENIDO"
					if(*flag)
					{
						asignarMarcosYTablas(contenido, package, tabla_proc,TLB);
						upPaginasAccedidas(tablaAccesos, package->PID);

						//log_info(logger, "Se hizo conexion con swap, se envio paquete a leer y este fue recibido correctamente");

						// Como la transferencia con el swap fue exitosa, le envio la pagina al CPU
						int tamanioMsj = strlen(contenido)+1;
						send(socketCliente,&tamanioMsj,sizeof(int),0);
						if (tamanioMsj >0)
							send(socketCliente, contenido, tamanioMsj, 0);
						log_info(logger,"Se informa al CPU confirmacion de lectura");

						upFallosPagina(tablaAccesos, package->PID);
					}
					else
					{
						int recibi= -1;
						send(socketCliente,&recibi,sizeof(int),0);
						log_error(logger, "Hubo un problema con la conexion/envio al swap. Se informa al CPU");
					}
				}else
				{
					mostrarVersus(tablaAccesos, package->PID);
					matarProceso(package, tabla_adm, TLB, tablaAccesos);
					int recibi= -1;
					send(socketCliente,&recibi,sizeof(int),0);
					log_info(logger, "Ya no tengo mas marcos disponibles en la memoria, rechazo pedido e informo al CPU");
				}

			}
		}else // SI NO ESTA EN SWAP, YA CONOZCO LA DIRECCION DE SU MARCO //
		{
			log_info(logger, "Se encontro la pagina a leer en memoria");
			usleep(miContexto.retardoMemoria * SLEEP); 	// SLEEP PORQUE OPERO CON LA PAGINA SEGUN ISSUE 71

			/*
			 * KOLOOO
			 * puedo llamar a actualizar proceso, en teoria si esta en FIFO o CLOCK no hace nada
			 *
			 */
			if(!strcmp(miContexto.algoritmoReemplazo, "LRU"))
			{
				actualizarTablaProcesoLru(tabla_proc, package->pagina_proceso, pagina_proc->direccion_fisica, pagina_proc->marco);
			}
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
					actualizarTlb(package->PID, package->pagina_proceso, pagina_proc->direccion_fisica, TLB, pagina_proc->marco);

		 	upPaginasAccedidas(tablaAccesos, package->PID );
		 	int tamanioMsj = strlen(pagina_proc->direccion_fisica)+1;
		 	send(socketCliente,&tamanioMsj,sizeof(int),0);

		 	log_info(logger, "La pagina contiene: %s. Se envia al CPU", pagina_proc->direccion_fisica);

		 	if (tamanioMsj >0)
		 		send(socketCliente, pagina_proc->direccion_fisica, tamanioMsj, 0);
		}
	}else
	{
		log_info(logger, "Se esta queriendo leer una pagina de un proceso que no esta iniciado, informo al CPU");
		int recibi= -1;
		send(socketCliente,&recibi,sizeof(int),0);
	}
}

void asignarMarcosYTablas(char * contenido, t_header * package, t_list* tabla_proc, t_list * TLB)
{
	t_marco_hueco * marco_a_llenar = list_remove(listaFramesHuecosMemR, 0);

	log_info(logger, "Traje la pagina del swap, voy a llenar el marco %d", marco_a_llenar->numero_marco);
	// SLEEP PORQUE OPERO CON LA PAGINA SEGUN ISSUE 71
	usleep(miContexto.retardoMemoria * SLEEP);

	// LO ESCRIBO CON EL MENSAJE QUE ME DICEN QUE LO ESCRIBA PORQUE NO TENGO QUE TRAER LO QUE YA ESTE ESCRITO DEL SWAP
	escribirMarco(contenido, marco_a_llenar->direccion_inicio, marco_a_llenar->numero_marco);


	//AGREGO EL MARCO AHORA ESCRITO, A LA LISTA DE MARCOS ESCRITOS
	list_add(listaFramesMemR, marco_a_llenar);

	//AGREGO LA PAGINA A LA TLB (VERIFICO SI ESTA LLENA Y REEMPLAZO)
	if (!strcmp(miContexto.tlbHabilitada, "SI"))
		actualizarTlb(package->PID, package->pagina_proceso, marco_a_llenar->direccion_inicio, TLB, marco_a_llenar->numero_marco);

	// ACTUALIZO LA TABLA DEL PROCESO CON LA DRIECCION FISICA, DEPENDIENDO EL ALGORITMO DEL CONTEXTO
	actualizoTablaProceso(tabla_proc, marco_a_llenar, package);

}

void escribirEnMemReal(t_list * tabla_adm, t_list * TLB, t_header * package, int serverSocket, int socketCliente, char * mensaje, t_list* tablaAccesos)
{
	int  * flag = malloc(sizeof(int));
	// SLEEP PORQUE LA MEMORIA BUSCA EN SUS ESTRUCTURAS
	usleep(miContexto.retardoMemoria * SLEEP);
	t_list * tabla_proc = obtenerTablaProceso(tabla_adm, package->PID);

	// SI ENCONTRO UN REGISTRO CON ESE PID
	if(tabla_proc!=NULL)
	{
		process_pag * pagina_proc= obtenerPaginaProceso(tabla_proc, package->pagina_proceso); // TRAIGO LA PAGINA BUSCADA

		// SI LA DIRECCION ES NULL ES PORQUE ESTA EN SWAP, SINO YA LA ENCONTRE EN MEMORIA
		if ( pagina_proc->direccion_fisica == NULL)
		{
			log_info(logger, "Se encontro la pagina para escribir en swap, se hace el pedido de lectura");
			if ( marcosProcesoLlenos(tabla_proc))
			{
				int verific=swapeando(tabla_proc,tabla_adm ,TLB, mensaje, serverSocket, package, tablaAccesos, socketCliente);
				if(verific)
				{
					bool recibi = true;
					send(socketCliente,&recibi,sizeof(bool),0);
					log_info(logger,"Se informa al CPU confirmacion de escritura");
				}
			}
			/* SI TENGO ESPACIO PARA TRAERLA (CANT MAX DE MARCOS PARA ESE PROCESO
			 *NO FUE ALCANZADA TODAVÍA), SI ME QUEDA MEMORIA (MARCOS) LA TRAIGO(MENTIRA)
			*/
			else
			{
				if ( listaFramesHuecosMemR->elements_count != 0)
				{
					char * contenido = malloc(miContexto.tamanioMarco);
					t_header * header_lectura = crearHeaderLectura(package);

					envioAlSwap(header_lectura, serverSocket, contenido, flag);

					if(*flag)
					{
						asignarMarcosYTablas(mensaje, package, tabla_proc,TLB);
						upPaginasAccedidas(tablaAccesos, package->PID);
						upFallosPagina(tablaAccesos, package->PID);

						// Como la transferencia con el swap fue exitosa, le envio el flag
						bool recibi= true;
						send(socketCliente,&recibi,sizeof(bool),0);
						log_info(logger,"Se informa al CPU confirmacion de escritura");
					}
					else
					{
						bool recibi=false;
						send(socketCliente,&recibi,sizeof(bool),0);
						log_error(logger, "Hubo un problema con la conexion/envio al swap");
					}
				}else
				{
					mostrarVersus(tablaAccesos, package->PID);
					matarProceso(package, tabla_adm, TLB, tablaAccesos);
					bool recibi= false ;
					send(socketCliente,&recibi,sizeof(bool),0);
					log_info(logger, "Ya no tengo mas marcos disponibles en la memoria, rechazo pedido e informo al CPU");
				}

			}
		}else // SI NO ESTA EN SWAP, YA CONOZCO LA DIRECCION DE SU MARCO //
		{
			log_info(logger, "Encontre PID: %d Pagina %d en memoria => tengo la direccion", package->PID, package->pagina_proceso);
			escribirMarco(mensaje, pagina_proc->direccion_fisica, pagina_proc->marco);

			if (!strcmp(miContexto.tlbHabilitada, "SI"))
				actualizarTlb(package->PID, package->pagina_proceso, pagina_proc->direccion_fisica, TLB, pagina_proc->marco);

			actualizoTablaProceso(tabla_proc, NULL, package);
			upPaginasAccedidas(tablaAccesos, package->PID );

			// ACTUALIZO EL BIT DIRTY A 1 POR HABER ESCRITO EN LA PAGINA QUE YA ESTABA EN MEMORIA
			if (!strcmp(miContexto.algoritmoReemplazo, "CLOCK"))
				pagina_proc->dirty = 1;
			bool recibi = true;
			send(socketCliente,&recibi,sizeof(bool),0);
			log_info(logger,"Se informa al CPU confirmacion de escritura");
		}
	}else
	{
		log_info(logger, "Se esta queriendo escribir una pagina de un proceso que no esta iniciado");
		bool recibi= false;
		send(socketCliente,&recibi,sizeof(bool),0);
	}
}

t_list * obtenerTablaProceso(t_list * tabla_adm, int pid)
{
	bool _numeroDePid (void * p){ return(*(int *)p == pid); }
	t_tabla_adm * reg_tabla_tablas = list_find(tabla_adm, (void*)_numeroDePid);

	// SI ENCONTRO UN REGISTRO CON ESE PID
	if (reg_tabla_tablas != NULL)
	{
		// TRAIGO LA TABLA DEL PROCESO Y LA DEVUELVO
		return reg_tabla_tablas->direc_tabla_proc;
	}
	return NULL;
}

process_pag * obtenerPaginaProceso(t_list * tabla_proceso, int pagina)
 {
	bool _numeroDePagina (void * p){ return(*(int*)p == pagina); }
 	 // TRAIGO LA PAGINA BUSCADA
	process_pag * pagina_proc = list_find(tabla_proceso, (void *)_numeroDePagina);
	return pagina_proc;
 }

void envioAlSwap ( t_header * header, int serverSocket, char * contenido, int * flag)
{
		send(serverSocket, header, sizeof(t_header), 0);
		/*
		 * Una vez enviado el registro, recibo la notificación por parte del swap.
	  	 * 0 = Hubo un error.
	  	 * 1 = Todo ok.
 	 	*/
		//SI EL TIPO DE EJECUCION ES ESCRITURA, MANDO EL CONTENIDO
		if (header->type_ejecution == 1)
		{
			send(serverSocket, contenido, miContexto.tamanioMarco, 0);
		}
		recv(serverSocket, flag, sizeof(int),0);

		if(*flag==1) //si no hubo error
		{
			if(header->type_ejecution==0) //si hice una lectura, devuelve la pag
			{
				recv(serverSocket, (void *)contenido, miContexto.tamanioMarco,0);
			}
		}
}

bool tlbLlena(t_list * TLB)
{
	int cant_elem = TLB->elements_count;
	if (cant_elem == miContexto.entradasTlb)
		return true;

	return false;
}

void matarProceso(t_header * proceso_entrante, t_list * tabla_adm, t_list * TLB, t_list* tablaAccesos)
{
	bool _numeroDePid (void * p){ return(*(int *)p == proceso_entrante->PID);	}
	t_list * tabla_proceso = obtenerTablaProceso(tabla_adm, proceso_entrante->PID);

	if (tabla_proceso != NULL)
	{
		int cantidad_paginas = tabla_proceso->elements_count;
		int x = 0;
		while (x < cantidad_paginas)
		{
			process_pag * pagina_removida = list_remove(tabla_proceso, 0);
			bool _numMarco (void * p){	return(*(char *)p == pagina_removida->marco);	}
			if ( pagina_removida->direccion_fisica != NULL)
			{	//ENCONTRO PAGINA A REMOVER QUE TENIA UN MARCO ASIGNADO
				removerMarcoPorMarco(pagina_removida->marco);
			}
			x++;
		}
			// ELIMINO LA ENTRADA DEL PROCESO EN LA TABLA DE ACCESOS
			list_remove_by_condition(tablaAccesos, (void*)_numeroDePid);
			// ELIMINO TODOS LOS ELEMENTOS DE LA TABLA Y LA TABLA
			list_destroy_and_destroy_elements(tabla_proceso, (void *)pag_destroy);
			//ELIMINO LA REFERENCIA DE ESA TABLA DE PROCESO, DESDE LA TABLA DE PROCESOS
			list_remove_and_destroy_by_condition(tabla_adm, (void*)_numeroDePid,(void*)tabla_adm_destroy);
			// ME FIJO SI EN LA TLB (SI ESTA HABILITADA) HABIA ALGUNA REFERENCIA A ALGUNA DE SUS PAGINAS Y LAS ELIMINO
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
				removerEntradasTlb(TLB, proceso_entrante);

	}
	else
	{
		log_error(logger, "No se encontro la tabla del proceso a remover");
	}
}

int removerEntradasTlb(t_list * TLB, t_header * header)
{
	bool _numeroDePid (void * p){ return(*(int *)p == header->PID);	}
	while ( list_find(TLB,(void*)_numeroDePid) != NULL )
	{
		list_remove_and_destroy_by_condition(TLB,(void*)_numeroDePid,(void*)reg_tlb_destroy);
	}
	return 1;
}

int escribirDesdeTlb (t_list * TLB, int tamanio_msg, char * message, t_header * pagina, t_list* tablaAccesos, t_list* tabla_adm, int socketCliente)
{
	int * posicion = malloc(sizeof(int));
	t_tlb * registro_tlb = buscarEntradaProcesoEnTlb(TLB, pagina, posicion);
	//Actualizo cantidad de accesos a TLB;
	cantAccesosTlb=cantAccesosTlb+1;
	if (registro_tlb != NULL)
	{
		upPaginasAccedidas(tablaAccesos, pagina->PID);
		log_info(logger, "TLB HIT pagina: %d en el marco numero: %d y dice: \"%s\"", registro_tlb->pagina,registro_tlb->marco, registro_tlb->direccion_fisica);
		//Actualizo cantidad de aciertos
		cantHitTlb= cantHitTlb +1;
		escribirMarco(message, registro_tlb->direccion_fisica, registro_tlb->marco);
		//strcpy (registro_tlb->direccion_fisica, message);

		/**************************************/
		// TRAIGO LA TABLA DEL PROCESO
		t_list * tablaProceso = obtenerTablaProceso(tabla_adm, pagina->PID);
		// TRAIGO LA PAGINA A ESCRIBIR ( SOLO EL NODO DE LA TABLA DEL PROCESO )
		process_pag * paginaProceso = obtenerPaginaProceso(tablaProceso, pagina->pagina_proceso);
		// ACTUALIZO EL BIT DIRTY A 1 POR HABER ESCRITO EN LA PAGINA QUE YA ESTABA EN MEMORIA
		if (!strcmp(miContexto.algoritmoReemplazo, "CLOCK"))
			paginaProceso->dirty = 1;
		actualizoTablaProceso(tablaProceso, NULL, pagina);

		bool recibi = true;
		send(socketCliente,&recibi,sizeof(bool),0);
		log_info(logger,"Se informa al CPU confirmacion de escritura");
		return 1;
	}else
	{
		// SI NO LA ENCONTRO RETORNO 0
		log_info(logger, "TLB MISS");
		return 0;
	}
}

t_tlb * buscarEntradaProcesoEnTlb (t_list * TLB, t_header * pagina, int * posicion)
{
	int x = 0;
	int tamanio_tlb = list_size(TLB);
	while ( x < tamanio_tlb)
	{
		t_tlb * reg_tlb = list_get(TLB, x);
		if (  reg_tlb->pid == pagina->PID)
		{
			if ( reg_tlb->pagina == pagina->pagina_proceso)
			{
				*posicion = x;
				return reg_tlb;
			}
		}
		x++;
	}
	*posicion = -1;
	return NULL;
}

int buscarIndicePunteroUno (t_list * lista_proceso)
{
	int x = 0;
	int tamanio_lista = list_size(lista_proceso);
	while ( x < tamanio_lista)
	{
		process_pag * reg = list_get(lista_proceso, x);
		if (  reg->puntero == 1)
		{
				return x;
		}
		x++;
	}
	return -1;
}

int buscarIndicePagina(t_list * tabla_paginas, process_pag * pagina)
{
	int x = 0;
	int tamanio = tabla_paginas->elements_count;
	while (x < tamanio)
	{
		process_pag * reg = list_get(tabla_paginas, x);
		if (reg->pag == pagina->pag)
		{
			return x;
		}
		x++;
	}
	return -1;
}

int buscaIndiceUltimaPagina(t_list* tabla_paginas)
{
	int x = 0;
	int tamanio = tabla_paginas->elements_count;
	while (x < tamanio)
	{
		process_pag * reg = list_get(tabla_paginas, x%tamanio);
		process_pag * reg2 = list_get(tabla_paginas, (x+1)%tamanio);
		if ( (reg->direccion_fisica != NULL) && (reg2->direccion_fisica == NULL) )
		{
			return x;
		}
		x++;
	}
	return -1;
}

int buscarIndicePrimerPagina(t_list* tabla_paginas)
{
	int x = 0;
	int tamanio = tabla_paginas->elements_count;
	while (x < tamanio)
	{
		process_pag * reg = list_get(tabla_paginas, x%tamanio);
		process_pag * reg2 = list_get(tabla_paginas, (x+1)%tamanio);
		if ( (reg->direccion_fisica == NULL) && (reg2->direccion_fisica != NULL) )
		{
			return x+1;
		}
		x++;
	}
	return -1;
}

int actualizarPunteroProximaPagina(t_list* tabla_paginas, int indice)
{
	int tamanio = tabla_paginas->elements_count;
	int y = 0;
	while (y < tamanio)
	{
		process_pag * pagina = list_get(tabla_paginas, (++indice)%tamanio);
		if ( pagina->direccion_fisica != NULL) // Si tiene un marco asignado
		{
			pagina->puntero = 1;
			return 1;
		}
		y++;
	}
	return -1;
}

int swapeando(t_list* tablaProceso,t_list* tabla_adm , t_list * TLB, char * mensaje, int serverSocket, t_header * header, t_list* tablaAccesos, int socketCliente)
{
	// paginaASwapear va a tener la pagina que ya esta en memoria y que se va a enviar al swap
	//header es la pagina que quiero leer de swap para escribir en el marco de paginaASwapear
	// TRAIGO LA PRIMER PAGINA QUE SE HAYA CARGADO EN MEMORIA
	process_pag * paginaASwapear = traerPaginaARemover(tablaProceso);
	log_info(logger, "Acceso a swap: Se swapea para traer la pagina %d porque no quedan marcos disponibles para el proceso %d", header->pagina_proceso, header->PID);
	log_info(logger, "Se va a remover la pagina: %d, que contiene: \"%s\"",paginaASwapear->pag, paginaASwapear->direccion_fisica);

	usleep(miContexto.retardoMemoria * SLEEP);

	t_header * header_escritura = crearHeaderEscritura( header->PID, paginaASwapear->pag, 0);

	int * status_escritura = malloc(sizeof(int));
	envioAlSwap(header_escritura, serverSocket, paginaASwapear->direccion_fisica, status_escritura);

	if ( *status_escritura != 1)
	{
		bool recibi=false;
		send(socketCliente,&recibi,sizeof(bool),0);
		log_error(logger, "No se pudo escribir en el Swap, aviso al CPU");
		return 0;
	}

	int num_pag;

	bool _numeroDePag (void * p){ return(*(int*)p == num_pag); }
	// SI SE TRATA DE UNA ESCRITURA
	if (header->type_ejecution == 1)
	{
		// LE PIDO LA PAGINA QUE QUIERO ESCRIBIR Y LA AGREGO AL FINAL DE LA LISTA
		t_header * header_lectura = crearHeaderLectura(header);

		int * status_lectura = malloc(sizeof(int));
		char * contenido = malloc(miContexto.tamanioMarco);
		envioAlSwap(header_lectura, serverSocket, contenido, status_lectura);

		num_pag = header->pagina_proceso;

		if (*status_lectura != 1)
		{
			log_error(logger, "No se pudo leer del Swap, aviso al CPU");
			bool recibi=false;
			send(socketCliente,&recibi,sizeof(bool),0);
			return 0;
		}

		log_info(logger, "Se escribe en el marco liberado la pagina que se quiere escribir****************");
		escribirMarco(mensaje, paginaASwapear->direccion_fisica, paginaASwapear->marco);

		upPaginasAccedidas(tablaAccesos, header->PID);
		upFallosPagina(tablaAccesos, header->PID);
		//bool recibi= true;
		//send(socketCliente,&recibi,sizeof(bool),0);

		// SI SE TRATA DE UNA LECTURA
	}else if(header->type_ejecution ==0)
	{
		int * status_lectura = malloc(sizeof(int));
		char * contenido = malloc(miContexto.tamanioMarco);
		envioAlSwap(header, serverSocket, contenido, status_lectura);

		if ( *status_lectura != 1)
		{
			int recibi = -1;
			send(socketCliente,&recibi,sizeof(int),0);
			log_error(logger, "No se pudo leer de swap, aviso al CPU");
			return 0;
		}

		log_info(logger, "Se escribe el marco liberado con la pagina recien traida del swap");
		// SLEEP PORQUE OPERO CON LA PAGINA SEGUN ISSUE 71
		usleep(miContexto.retardoMemoria * SLEEP);
		escribirMarco(contenido, paginaASwapear->direccion_fisica, paginaASwapear->marco);

		int tamanioMsj = strlen(contenido)+1;
		send(socketCliente,&tamanioMsj,sizeof(int),0);
		if (tamanioMsj >0)
			send(socketCliente, contenido, tamanioMsj, 0);
		log_info(logger, "Se envia al CPU confirmacion de lectura");

		upPaginasAccedidas(tablaAccesos, header->PID);
		upFallosPagina(tablaAccesos, header->PID);
	}
	num_pag = paginaASwapear->pag;

	if (!strcmp(miContexto.tlbHabilitada, "SI"))
	{
		int * posicion = malloc(sizeof(int));
		t_header * headerParaTlb =  package_create(0, header->PID, paginaASwapear->pag, 0);
		buscarEntradaProcesoEnTlb(TLB, headerParaTlb ,posicion);

		// SI LA ENCONTRO EN LA TLB
		if (*posicion != -1)
		{
			log_info(logger, "Borro la entrada de la TLB de la posicion-->%d", *posicion);
			list_remove(TLB, *posicion);
		}
		actualizarTlb(header->PID, header->pagina_proceso, paginaASwapear->direccion_fisica, TLB, paginaASwapear->marco);
	}

	//ACTUALIZO LA TABLA DEL PROCESO SEGUN ALGORITMOS // NO USO ACTUALIZOTABLAPROCESO POR LOS PARAMETROS
	if ( !strcmp(miContexto.algoritmoReemplazo, "FIFO"))
	{
		actualizarTablaProcesoFifo(tablaProceso, header->pagina_proceso, paginaASwapear->direccion_fisica, paginaASwapear->marco);
	}else if (!strcmp(miContexto.algoritmoReemplazo, "LRU")) // LA MUE
	{
		actualizarTablaProcesoLru(tablaProceso, header->pagina_proceso, paginaASwapear->direccion_fisica, paginaASwapear->marco);
	}else // CLOCK POR DESCARTE YA QUE NO PUEDE HABER ERRORES EN EL ARCHIVO DE CONFIGURACION, ACTUALIZA EL BIT DE ACCEDIDO A 1
	{
		actualizarTablaProcesoClock(tablaProceso, header, paginaASwapear->direccion_fisica, paginaASwapear->marco, 0);
	}

	// ACTUALIZO LA PAGINA QUE SWAPEE ( LA ELIMINO Y LA VUELVO A AGREGAR VACIA )
	if ( strcmp(miContexto.algoritmoReemplazo, "CLOCK") )
	{
		list_remove_by_condition(tablaProceso, (void*)_numeroDePag);
		list_add(tablaProceso, pag_proc_create(paginaASwapear->pag, NULL, -1, 0, 0, 0));
	}
	return 1;
}

void actualizoTablaProceso(t_list * tablaProceso, t_marco_hueco * marco_a_llenar , t_header * header)
{
	if(!strcmp(miContexto.algoritmoReemplazo, "FIFO"))
	{
		log_info(logger, "Actualizando tabla corte FIFO");
		if(marco_a_llenar!=NULL)
			actualizarTablaProcesoFifo(tablaProceso, header->pagina_proceso, marco_a_llenar->direccion_inicio, marco_a_llenar->numero_marco);
		else
			actualizarTablaProcesoFifo(tablaProceso, header->pagina_proceso, NULL, NULL);
	}else if (!strcmp(miContexto.algoritmoReemplazo, "LRU"))
	{
		log_info(logger, "Actualizando tabla corte LRU");
		if(marco_a_llenar!=NULL)
			actualizarTablaProcesoLru(tablaProceso, header->pagina_proceso, marco_a_llenar->direccion_inicio, marco_a_llenar->numero_marco);
		else
			actualizarTablaProcesoLru(tablaProceso, header->pagina_proceso, NULL, NULL);
	}else
	{
		log_info(logger, "Actualizando tabla corte CLOCK");
		if(marco_a_llenar!=NULL)
			actualizarTablaProcesoClock(tablaProceso, header, marco_a_llenar->direccion_inicio, marco_a_llenar->numero_marco, 1);
		else
			actualizarTablaProcesoClock(tablaProceso, header, NULL, NULL, 1);
	}
}

void actualizarTablaProcesoLru(t_list * tabla_proceso, int num_pagina, char * direccion_marco, int num_marco)
{
	bool _numeroDePagina (void * p)	{	return(*(int*)p == num_pagina);	}

	process_pag * pag = list_remove_by_condition(tabla_proceso, (void*)_numeroDePagina);
	process_pag * pagina;
	if(direccion_marco!=NULL)
		pagina = pag_proc_create(num_pagina, direccion_marco, num_marco, 0, 0, 0);
	else
		pagina = pag_proc_create(num_pagina, pag->direccion_fisica, pag->marco, 0, 0, 0);
	list_add(tabla_proceso,pagina);
}

void actualizarTablaProcesoFifo(t_list * tabla_proceso, int num_pagina, char * direccion_marco, int num_marco)
{
	bool _numeroDePagina (void * p){ return(*(int*)p == num_pagina);}

	process_pag * pagina = list_find(tabla_proceso, (void*)_numeroDePagina);
	if( pagina->direccion_fisica == NULL)
	{
		list_remove_by_condition(tabla_proceso, (void*)_numeroDePagina);
		pagina->direccion_fisica = direccion_marco;
		pagina->marco = num_marco;
		list_add(tabla_proceso, pagina);
	}else	{ // TECNICAMENTE SI YA ESTABA CARGADA, LA DEJO EN LA POSICION EN LA QUE ESTABA
	}

}

void actualizarTablaProcesoClock(t_list * tabla_proceso, t_header * header, char * direccion_marco, int num_marco, int modo)
{
	int numPag = header->pagina_proceso;
	int tamanio = tabla_proceso->elements_count;
	bool _numeroDePagina (void * p) {return(*(int *)p == numPag);}
	int indicePuntero;

	int indiceUltPag = buscaIndiceUltimaPagina(tabla_proceso); 	// Indice de la ultima pagina que tiene asignado un marco
	int indicePriPag = buscarIndicePrimerPagina(tabla_proceso); 	// Indice de la primer pagina que tiene asignado un marco

	process_pag * pagina = list_find(tabla_proceso, (void*)_numeroDePagina);

	switch (modo)
	{
		case 0: // SI HAY QUE REMOVER PAGINA (SWAPEANDO) Falta actualizar puntero
			if( pagina->direccion_fisica == NULL)
			{
				pagina = list_remove_by_condition(tabla_proceso, (void*)_numeroDePagina);

				process_pag * paginaASwapear = traerPaginaARemover(tabla_proceso);

				// Obtengo el indice de la pagina que voy a eliminar
				indicePuntero = buscarIndicePagina(tabla_proceso, paginaASwapear);
				// Apunto a la pagina que corresponde apuntar
				actualizarPunteroProximaPagina(tabla_proceso, indicePuntero);

				numPag = paginaASwapear->pag;
				process_pag * elem_apuntado = list_remove_by_condition(tabla_proceso, (void*)_numeroDePagina);

				// Actualizo los valores de la nueva pagina
				pagina->direccion_fisica = elem_apuntado->direccion_fisica;
				pagina->marco = elem_apuntado->marco;
				pagina->accessed = 1;

				// Limpio los valores de la pagina que ahora está en swap
				elem_apuntado->direccion_fisica = NULL;
				elem_apuntado->marco = -1;
				elem_apuntado->dirty = 0;
				elem_apuntado->accessed = 0;
				elem_apuntado->puntero = 0; // Por las dudas

				//Agrego la nueva pagina en la posición en donde estaba la pagina que swapee
				list_add_in_index(tabla_proceso, indicePuntero, pagina);
				if ( indiceUltPag == -1) // Si quedaron todas las que tienen un marco, agrego la vacia a lo ultimo
					list_add(tabla_proceso, elem_apuntado);
				else // Agrego la pagina que esta ahora en swap, al final con los valores limpios
					list_add_in_index(tabla_proceso, indiceUltPag, elem_apuntado);
			}else
			{
				puts("ERROR CLOCK");
			}
			break;
		case 1: // LOS DEMAS CASOS
			if ( pagina->direccion_fisica != NULL) // Si ya esta en memoria
			{
				// No corro el puntero, actualizo el bit de accedido (afuera del if)
			}else // Si la trajo de swap y le tengo que asignar un marco y demas
			{
				pagina = list_remove_by_condition(tabla_proceso, (void*)_numeroDePagina);
				pagina->direccion_fisica = direccion_marco;
				pagina->marco = num_marco;
				if (indiceUltPag != -1) // Si hay alguna cargada
				{
					list_add_in_index(tabla_proceso,(indiceUltPag+1)%tamanio, pagina);
				}else // Si no hay ninguna cargada la agrego al final y le asigno el puntero
				{
					list_add(tabla_proceso, pagina);
					pagina->puntero = 1;
				}
			}
			break;
		default:
			puts("DEFAULT \n");
	}

	pagina->accessed = 1;
	if(header->type_ejecution == 1)
		pagina->dirty =1;
}

void actualizarPunteroClock(t_list * tabla_proceso, int num_pag)
{
	bool _numeroDePagina (void * p)	{	return(*(int*)p == num_pag);	}

  	int x = 0;
  	int tam = tabla_proceso->elements_count;

  	while (x < tam)
  	{
  		process_pag * pagina = list_get(tabla_proceso, x);
  		if (pagina->puntero == 1)
  			pagina->puntero = 0;

  	x++;
  	}
  	process_pag * pag = list_find(tabla_proceso, (void*)_numeroDePagina);
  	pag->puntero = 1;
}

process_pag * traerPaginaARemover(t_list * tablaProceso)
{
	// SI ALGORITMO == FIFO O == LRU
	if (!strcmp(miContexto.algoritmoReemplazo, "FIFO") || (!strcmp(miContexto.algoritmoReemplazo, "LRU")) )
	{
		return paginaARemoverFifoLru(tablaProceso);
	// SI ALGIRTMO == CLOCK
	}else
	{
		return paginaARemoverClock(tablaProceso);
	}
	return NULL;
}

process_pag * paginaARemoverFifoLru(t_list * tablaProceso)
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
	// SI NO HAY NINGUNA CARGADA, O SEA, SI NO TIENE NADA PARA SWAPEAR
	return NULL;
}

process_pag * paginaARemoverClock(t_list * tablaProceso)
{
	process_pag * pagina = ambosBitsEnCero(tablaProceso);
	// SI NO ENCONTRE UNA PAGINA CON AMBOS BITS EN 0
	if ( pagina == NULL)
	{
		pagina= bitDirtyEnUno(tablaProceso);
		if ( pagina == NULL)
		{
			pagina = ambosBitsEnCero(tablaProceso);
			if (pagina == NULL)
				return bitDirtyEnUno(tablaProceso);
			else
				return pagina;
		}else
			return pagina;

	// SI ENCONTRE UNA PAGINA CON AMBOS BITS EN CERO, LA RETORNO
	}else
	{
		return pagina;
	}
}

process_pag * ambosBitsEnCero(t_list * tablaProceso)
{
	int tamanio = tablaProceso->elements_count;
	int x = buscarIndicePunteroUno(tablaProceso);
	int y = 0;
	while ( y < tamanio)
	{
		process_pag * pagina = list_get(tablaProceso, x%tamanio);

		if ( (pagina->direccion_fisica != NULL) && (pagina->accessed == 0) && (pagina->dirty == 0) )
		{
			return pagina;
		}
		x++;
		y++;
	}
	return NULL;
}

process_pag * bitDirtyEnUno (t_list * tablaProceso)
{
	int tamanio = tablaProceso->elements_count;
	int x = buscarIndicePunteroUno(tablaProceso);
	int y = 0;
	while ( y < tamanio)
	{
		process_pag * pagina = list_get(tablaProceso, x%tamanio);
		if( (pagina->direccion_fisica != NULL) && (pagina->accessed == 0) && (pagina->dirty == 1) )
		{
			return pagina;
		}else if ( ( pagina->accessed == 1 && pagina->dirty == 0) || ( pagina->accessed == 1 && pagina->dirty == 1) )
		{
			pagina->accessed = 0;
		}
		y++;
		x++;
	}
	return NULL;
}

void escribirMarco(char * mensaje, char* direccion, int marco)
{
	usleep(miContexto.retardoMemoria * SLEEP);
	char * pagAux = calloc(1, miContexto.tamanioMarco); // sobreescribo pagina con \0
	strcpy(direccion, pagAux);
	int tam_msj = strlen(mensaje);
	if (tam_msj >= miContexto.tamanioMarco )
	{
		mensaje[miContexto.tamanioMarco-1] = '\0';
		memcpy(direccion, mensaje, miContexto.tamanioMarco); // y despues le escribo el mensaje
	}else
	{
		mensaje[tam_msj] = '\0';
		memcpy(direccion, mensaje, tam_msj+1);
	}

	log_info(logger,"Se escribio en el marco: %d, el contenido: \"%s\"",marco,direccion);
}

int marcosProcesoLlenos(t_list * lista_proceso)
{
	int x = 0;
	int paginas_ocupadas = 0;
	int paginas_disponibles = 0;
	int cantidad_paginas = lista_proceso->elements_count;
	while ( paginas_ocupadas+paginas_disponibles < cantidad_paginas )
	{
		process_pag * reg = list_get(lista_proceso, x);
		if ( reg->direccion_fisica != NULL )
			paginas_ocupadas++;
		else
			paginas_disponibles++;

		x++;
	}

	if (paginas_ocupadas == miContexto.maxMarcos)
		return 1;
	else
		return 0;
}

void actualizarTlb (int pid, int pagina, char * direccion_memoria, t_list * TLB, int marco)
{
	int posicion;
	t_tlb * entrada_tlb = buscarEntradaProcesoEnTlb(TLB, package_create(0,pid,pagina,0), &posicion);

	if( entrada_tlb == NULL)
	{
		// SI TENGO ESPACIO EN LA TLB, AGREGO UNA ENTRADA
		if (miContexto.entradasTlb != TLB->elements_count)
		{
			list_add(TLB, reg_tlb_create(pid, pagina, direccion_memoria, marco));
			// SI ESTA LLENA, REMUEVO EL PRIMER ELEMENTO Y AGREGO EL RECIEN USADO AL FINAL
		}else
		{
			t_tlb * entrada_removida = list_remove(TLB, 0);

			log_info(logger, "Se elimina la entrada de la TLB con pagina-> %d del proceso con pid-> %d \n"
							"Se agrega una entrada para la pagina-> %d de pid-> %d",
								entrada_removida->pagina, entrada_removida->pid, pagina, pid);
			list_add(TLB, reg_tlb_create(pid, pagina, direccion_memoria, marco ));
		}
		log_info(logger, "TLB actualizada, se agrego entrada para la pagina %d", pagina);
	}
}

void upPaginasAccedidas(t_list* tablaAccesos, int pid)
{
	bool _numeroDePid (void * p){ return(*(int *)p == pid);}

	t_versus * reg = list_find(tablaAccesos, (void*)_numeroDePid);
	reg->cantPagAccessed++;

	log_info(logger, "--------------------------------------ACCESO------------------------------------------------");

}

void upFallosPagina(t_list* tablaAccesos, int pid)
{
	bool _numeroDePid (void * p){ return(*(int *)p == pid); }

	t_versus * reg = list_find(tablaAccesos, (void*)_numeroDePid);
	reg->cantFallosPag++;

	log_info(logger, "--------------------------------------FALLO------------------------------------------------");
}

void mostrarVersus(t_list* tablaAccesos, int pid)
{
	bool _numeroDePid (void * p){ return(*(int *)p == pid);	}

	t_versus * reg = list_find(tablaAccesos, (void*)_numeroDePid);

	log_info(logger, "Cantidad de paginas accedidas-> %d VS Cantidad de fallos de pagina-> %d", reg->cantPagAccessed, reg->cantFallosPag);
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

void removerMarcoPorMarco(int marco)
{
	int cant_marcos = listaFramesMemR->elements_count;
	int x = 0;
	// INICIO UN PUNTERO LLENO DE CEROS PARA LIMPIAR LOS MARCOS OCUPADOS QUE PASO A LA LISTA DE HUECOS
	char * marcoAux = calloc(1, miContexto.tamanioMarco);

	while ( x < cant_marcos)
	{
		t_marco_hueco * marco_reg = list_get(listaFramesMemR, 0);
		if ( marco_reg->numero_marco == marco)
		{
			t_marco_hueco * marco_a_remover = list_remove(listaFramesMemR, x);

			strcpy(marco_a_remover->direccion_inicio, marcoAux); // limpio el marco para mandarlo a la lista de marcos vacios

			//AGREGO EL MARCO AHORA HUECO, A LA LISTA DE MARCOS HUECOS
			// SIGUE TENIENDO SU DIRECCION Y SU NUMERO DE MARCO, NO IMPORTA EN QUE LISTA ESTE
			list_add(listaFramesHuecosMemR, marco_a_remover);
		}
		x++;
	}
}

//------SEÑALES QUE TIENE QUE RECIBIR LA MEMORIA-------------//
void tlbFlush(t_list * TLB)
{
	if (!strcmp(miContexto.tlbHabilitada,"SI"))
	{
		int i=0;
		int cant_elementos = TLB->elements_count;
		for(;i<cant_elementos;i++)
		{
			list_remove_and_destroy_element(TLB, 0,(void*)reg_tlb_destroy);
		}
		puts("Se vacio la TLB \n");
	}
	else
	{
		puts("La TLB NO esta habilitada. \n");
	}
}

void limpiarMemoria(char * memoria_real, t_list * TLB, t_list * tabla_adm, int serverSocket)
{
	puts("Empiezo a limpiar la memoria");
	//Vacio la memoria
	int k = 0;
	char * punteroAMemoria=  memoria_real;

	//Actualizo las listas
	int i = 0, j = 0;
	for(;i<tabla_adm->elements_count;i++) //Recorro la tabla de tablas
	{
		puts("Actualizo listas");
		//Traigo una tabla
		t_tabla_adm * entrada_tabla_tablas = list_get(tabla_adm,i);
		t_list * tablaProceso = entrada_tabla_tablas->direc_tabla_proc;
		for(;j<tablaProceso->elements_count;j++) //Recorro la tabla de procesos
		{
			process_pag * pagina_proc = list_get(tablaProceso, j); //Traigo una pagina

			if(pagina_proc->direccion_fisica!=NULL) //Se la mando a escribir al swap.
			{

				t_header * header_escritura = crearHeaderEscritura(entrada_tabla_tablas->pid, pagina_proc->pag, 0);

				int * status_escritura = malloc(sizeof(int));
				envioAlSwap(header_escritura, serverSocket, pagina_proc->direccion_fisica, status_escritura);

				if ( *status_escritura != 1)
				{
					log_error(logger, "No se pudo escribir en el Swap.");
				}

			}
			//Actualizo la pagina
			pagina_proc->direccion_fisica=NULL;
			pagina_proc->marco=-1;
			pagina_proc->dirty=0;
			pagina_proc->accessed=0;
		}
	}

	while(k<miContexto.cantidadMarcos*miContexto.tamanioMarco)
	{
		strcpy(punteroAMemoria,"");
		punteroAMemoria++;
		k++;
	}

	if(tabla_adm->elements_count==0)
	{
	}
	//Actualizo marcos
	puts("Actualizo marcos");
	list_destroy_and_destroy_elements(listaFramesMemR,(void *)marco_destroy);
	list_destroy_and_destroy_elements(listaFramesHuecosMemR,(void *)marco_hueco_destroy);
	listaFramesMemR = crearListaFrames();
	listaFramesHuecosMemR = crearListaHuecosFrames(miContexto.cantidadMarcos, miContexto.tamanioMarco, memoria_real);
	tlbFlush(TLB);
}

void dumpEnLog(char * memoria_real, t_list * tablaAdm)
{
	int i = 0, j = 0;
	for(;i<tablaAdm->elements_count;i++) //Recorro la tabla de tablas
	{
		//Traigo una tabla
		t_tabla_adm * entrada_tabla_tablas = list_get(tablaAdm,i);
		t_list * tablaProceso = entrada_tabla_tablas->direc_tabla_proc;
		for(;j<tablaProceso->elements_count;j++) //Recorro la tabla de procesos
		{
			process_pag * pagina_proc = list_get(tablaProceso, j); //Traigo una pagina
			if(pagina_proc->marco!=-1) //si el marco no es -1 es porque esta cargado en un marco
			{
					log_info(logger,"Marco: %d ; Contenido: \"%s\"",pagina_proc->marco, pagina_proc->direccion_fisica);
			}
		}
	}
	if(tablaAdm->elements_count==0)
	{
		log_info(logger,"La memoria esta vacia");
	}
}
void tasasDeTLB()
{
	float porcentajeHit = 0;
	if(cantAccesosTlb!=0)
		porcentajeHit = (cantHitTlb * 100)/cantAccesosTlb;
	log_info(logger, "La cantidad de accesos a la TLB es: %d \n", cantAccesosTlb);
	log_info(logger, "La cantidad de aciertos de la TLB es: %d \n", cantHitTlb);
	log_info(logger, "La tasa de aciertos de la TLB es: %f%\n", porcentajeHit);
}
//-----------------------------------------------------------//

// --------------ENTRADAS A LA TABLA DE PROCESO ------------ //
process_pag * pag_proc_create (int pagina, char * direccion_fisica, int marco, int accedido, int dirty, int puntero)
{
 process_pag * reg_pagina = malloc(sizeof(process_pag));
 reg_pagina->pag = pagina;
 reg_pagina->direccion_fisica = direccion_fisica;
 reg_pagina->marco = marco;
 reg_pagina->accessed = accedido;
 reg_pagina->dirty = dirty;
 reg_pagina->puntero = puntero;
 return reg_pagina;
}

void pag_proc_destroy(process_pag * self)
{
	free(self);
}
// --------------------------------------------------------//

