/*
 * funcMemory.c
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */
//#include <SharedLibs/manejoListas.h>
#include "funcMemory.h"

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
	printf ("Memoria Liberada");
}

void ejecutoInstruccion(t_header * registro_prueba, char * mensaje,char *  memoria_real, t_list * TLB, t_list * tabla_adm, int socketCliente, int serverSocket, t_list* tablaAccesos)
{
	int * flag = malloc(sizeof(int));
	// DEPENDIENDO EL TIPO DE EJECUCION
	switch (registro_prueba->type_ejecution)
	{
	 	case 0:
	 		log_info(logger, "Solicitud de lectura recibida del PID: %d y pagina: %d", registro_prueba->PID, registro_prueba->pagina_proceso);
			/* CUANDO SE RECIBE UNA INSTRUCCION DE LECTURA, PRIMERO SE VERIFICA LA TLB A VER SI YA FUE CARGADA
			 * RECIENTEMENTE, EN CASO CONTRARIO SE ENVIA AL SWAP, ESTE ME VA A DEVOLVER LA PAGINA A LEER,
			 * LA MEMORIA LA ALMACENA EN UN MARCO DISPONIBLE PARA EL PROCESO DE LA PAGINA Y ACTUALIZA SUS TABLAS
			 */

			// PRIMERO VERIFICO QUE LA TLB ESTE HABILITADA*/
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
			{
				pthread_mutex_lock (&mutexTLB);
				int flagg=leerDesdeTlb(socketCliente,TLB, registro_prueba->PID, registro_prueba->pagina_proceso, tablaAccesos);
				pthread_mutex_unlock (&mutexTLB);
				// SI NO ESTABA EN TLB, ME FIJO EN MEMORIA
				if(!flagg)
				{
					pthread_mutex_lock (&mutexMem);
					leerEnMemReal(tabla_adm,TLB, registro_prueba, serverSocket,socketCliente, memoria_real, tablaAccesos);
					pthread_mutex_unlock (&mutexMem);
				}
			}
			else
			{
				pthread_mutex_lock (&mutexMem);
				leerEnMemReal(tabla_adm, TLB,registro_prueba, serverSocket,socketCliente, memoria_real, tablaAccesos);
				pthread_mutex_unlock (&mutexMem);
			}
	 		break;
	 	case 1:
			log_info(logger, "Solicitud de escritura recibida del PID: %d y pagina: %d", registro_prueba->PID, registro_prueba->pagina_proceso);
			int okTlb=0;
			// DECLARO UN FLAG PARA SABER SI ESTABA EN LA TLB Y SE ESCRIBIO, O SI NO ESTABA
			pthread_mutex_lock (&mutexTLB);
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
				okTlb = verificarTlb(TLB,registro_prueba->tamanio_msj, mensaje, registro_prueba, tablaAccesos);
			pthread_mutex_unlock (&mutexTLB);

			// SI ESTABA EN LA TLB, YA LA FUNCION ESCRIBIO Y LISTO
			if(okTlb == 1)
			{
				pthread_mutex_lock (&mutexMem);
				// TRAIGO LA TABLA DEL PROCESO
				t_list * tablaProceso = obtenerTablaProceso(tabla_adm, registro_prueba->PID);
				// TRAIGO LA PAGINA A ESCRIBIR ( SOLO EL NODO DE LA TABLA DEL PROCESO )
				process_pag * paginaProceso = obtenerPaginaProceso(tablaProceso, registro_prueba->pagina_proceso);
				pthread_mutex_unlock (&mutexMem);

				// ACTUALIZO EL BIT DIRTY A 1 POR HABER ESCRITO EN LA PAGINA QUE YA ESTABA EN MEMORIA
				if (!strcmp(miContexto.algoritmoReemplazo, "CLOCK"))
					paginaProceso->dirty = 1;
				printf ("SE ESCRIBIO CORRECTAMENTE PORQUE ESTABA EN LA TLB \n");
				bool recibi = true;
				send(socketCliente,&recibi,sizeof(bool),0);
				// PAGINA ACCEDIDA
			}
			// SI NO ESTABA EN LA TLB, AHORA ME FIJO SI ESTA EN LA TABLA DE TABLAS
			else
			{
				pthread_mutex_lock (&mutexMem);
				// TRAIGO LA TABLA DEL PROCESO
				t_list * tablaProceso = obtenerTablaProceso(tabla_adm, registro_prueba->PID);
				// HAGO UN SLEEP PORQUE LA MEMORIA VA A VERIFICAR SUS ESTRUCTURAS, SEGUN ISSUE #71
				sleep(miContexto.retardoMemoria);

				if(tablaProceso == NULL) // no tendria que entrar nunca aca porque supuestamente el archivo de instrucciones no tiene errores
				{
					log_error(logger, "Este proceso fue finalizado recientemente o nunca se inicio");
					break;
				}
				// TRAIGO LA PAGINA A ESCRIBIR ( SOLO EL NODO DE LA TABLA DEL PROCESO )
				process_pag * paginaProceso = obtenerPaginaProceso(tablaProceso, registro_prueba->pagina_proceso);

				// SI ESTA EN SWAP
				if ( paginaProceso->direccion_fisica == NULL)
				{
					log_info(logger, "PAGE FAULT, PID = %d y PAGINA = %d", registro_prueba->PID, registro_prueba->pagina_proceso);
					/* SI NO TENGO ESPACIO PARA TRAERLA (TODOS LOS MARCOS DISPONIBLES PARA ESE
					 * PROCESO YA ESTAN LLENOS), SWAPEO LA PRIMER PAGINA CARGADA (FIFO)
					 * Y ESCRIBO LA QUE RECIBO DEL SWAP AL FINAL DE LA LISTA. ACTUALIZO TLB
					 */
					if ( listaFramesHuecosMemR->elements_count != 0)
					{
						if ( marcosProcesoLlenos(tablaProceso))
						{

							int verific = swapeando(tablaProceso,tabla_adm ,TLB, mensaje, serverSocket, registro_prueba, tablaAccesos);
							if (verific == 1)
							{
								upFallosPagina(tablaAccesos, registro_prueba->PID);
								upPaginasAccedidas(tablaAccesos, registro_prueba->PID);
								puts(" KKKKKKKKKKKKKKK HICE UN FALLO KKKKKKKKKKKKK");
								bool recibi = true;
								send(socketCliente,&recibi,sizeof(bool),0);
							}
							else
							{
								puts(" NOVerifico");
								bool recibi = false;
								send(socketCliente,&recibi,sizeof(bool),0);
								log_error(logger, "Error al intentar swapear");
							}


						}
						else
						{
							/* SI TENGO ESPACIO PARA TRAERLA (CANT MAX DE MARCOS PARA ESE PROCESO
							 *NO FUE ALCANZADA TODAVÍA), SI ME QUEDA MEMORIA (MARCOS) LA TRAIGO(MENTIRA) Y LA ESCRIBO
							 */
							t_marco_hueco * marco_a_llenar = list_remove(listaFramesHuecosMemR, 0);
							log_info(logger, "Traje la pagina del swap, voy a escribir el marco %d", marco_a_llenar->numero_marco);
							sleep(miContexto.retardoMemoria);
							// LO ESCRIBO CON EL MENSAJE QUE ME DICEN QUE LO ESCRIBA PORQUE NO TENGO QUE TRAER LO QUE YA ESTE ESCRITO DEL SWAP
							memcpy ( marco_a_llenar->direccion_inicio, mensaje, strlen(mensaje));
							//AGREGO EL MARCO AHORA ESCRITO, A LA LISTA DE MARCOS ESCRITOS
							list_add(listaFramesMemR, marco_a_llenar);
							t_marco_hueco * asd = list_get(listaFramesMemR, 0);
							printf("***************************** EL MARCO NUM -> %d, TIENE--> %p \n", asd->numero_marco, asd->direccion_inicio);

							//AGREGO LA PAGINA A LA TLB (VERIFICO SI ESTA LLENA Y REEMPLAZO)
							pthread_mutex_lock (&mutexTLB);
							if (!strcmp(miContexto.tlbHabilitada, "SI"))
									actualizarTlb(registro_prueba->PID, registro_prueba->pagina_proceso, marco_a_llenar->direccion_inicio, TLB, marco_a_llenar->numero_marco);
							pthread_mutex_unlock (&mutexTLB);
							// ACTUALIZO LA TABLA DEL PROCESO CON LA DRIECCION FISICA, DEPENDIENDO EL ALGORITMO DEL CONTEXTO
							actualizoTablaProceso(tablaProceso, marco_a_llenar, registro_prueba);

							// Aumento 1 la cantidad de paginas accedidas del proceso
							upPaginasAccedidas(tablaAccesos, registro_prueba->PID );
							// Aumento 1 la cantidad de fallos de pagina porque la fui a buscar al swap
							upFallosPagina(tablaAccesos, registro_prueba->PID);
							puts(" KKKKKKKKKKKKKKK HICE UN FALLO KKKKKKKKKKKKK");

							// SI NO ME QUEDAN MARCOS EN TODA LA MEMORIA PARA GUARDAR UNA PAGINA, CHAU
							bool recibi = true;
							send(socketCliente,&recibi,sizeof(bool),0);
						}
					}
					else
					{
						// EL PROCESO TIENE CARGADO AL MENOS UNA PAGINA? LA SWAPEO
						if (procesoTienePaginaCargada(tablaProceso))
						{
							int verificar = swapeando(tablaProceso,tabla_adm ,TLB, mensaje, serverSocket, registro_prueba, tablaAccesos);
							if(verificar)
							{
								upFallosPagina(tablaAccesos, registro_prueba->PID);
								puts(" KKKKKKKKKKKKKKK HICE UN FALLO KKKKKKKKKKKKK");
								bool recibi = true;
								send(socketCliente,&recibi,sizeof(bool),0);
							}
							else{
								puts("No Verifico");
								bool recibi = false;
								send(socketCliente,&recibi,sizeof(bool),0);
							}
						}else
						{
							// POR ISSUE #25, SE FINALIZA EL PROCESO AUNQUE TENGA LUGAR RESPECTO A SU CANTIDAD
							// MAXIMA DE MARCOS, SI NO HAY MÁS MARCOS PARA ASIGNAR
							log_info(logger, "Ya no tengo mas marcos disponibles en la memoria, rechazo pedido");
							mostrarVersus(tablaAccesos, registro_prueba->PID);
							matarProceso(registro_prueba, tabla_adm, TLB, tablaAccesos);
							bool recibi = false;
							send(socketCliente,&recibi,sizeof(bool),0);
						}
						//upFallosPagina(tablaAccesos, registro_prueba->PID);
					}
				// SI NO ESTA EN SWAP, ENTONCES okMem TIENE LA DIRECCION DEL MARCO PARA ESCRIBIR EL MENSAJE
				}else
				{
					log_info(logger, "Encontre la pagina en memoria => tengo la direccion y la escribo");
					sleep(miContexto.retardoMemoria);
					memcpy ( paginaProceso->direccion_fisica, mensaje, registro_prueba->tamanio_msj);
					pthread_mutex_lock (&mutexTLB);
					if (!strcmp(miContexto.tlbHabilitada, "SI"))
						actualizarTlb(registro_prueba->PID, registro_prueba->pagina_proceso, paginaProceso->direccion_fisica, TLB, paginaProceso->marco);
					pthread_mutex_unlock (&mutexTLB);
					actualizoTablaProceso(tablaProceso, NULL, registro_prueba);
					upPaginasAccedidas(tablaAccesos, registro_prueba->PID );

					// ACTUALIZO EL BIT DIRTY A 1 POR HABER ESCRITO EN LA PAGINA QUE YA ESTABA EN MEMORIA
					if (!strcmp(miContexto.algoritmoReemplazo, "CLOCK"))
						paginaProceso->dirty = 1;

					bool recibi = true;
					send(socketCliente,&recibi,sizeof(bool),0);
				}
				pthread_mutex_unlock (&mutexMem);
			}
	 		break;
	 	case 2:
	 		/* LA INICIALIZACION SE MANDA DIRECO AL SWAP PARA QUE RESERVE ESPACIO,
	 		   EL FLAG = 1 ME AVISA QUE RECIBIO OK */
	 		envioAlSwap(registro_prueba, serverSocket, NULL, flag);
	 		bool recibi;
	 		if(*flag == 1)
	 		{
	 			//creo todas las estructuras porque el swap ya inicializo
	 			pthread_mutex_lock (&mutexMem);
	 			iniciarProceso(tabla_adm, registro_prueba, tablaAccesos);
	 			pthread_mutex_unlock (&mutexMem);
	 			log_info(logger, "Proceso mProc creado, numero de PID: %d y cantidad de paginas: %d"
	 											,registro_prueba->PID, registro_prueba->pagina_proceso);
	 			// VER COMO MANDAR LA VALIDACION AL CPU QUE NO LE ESTA LLEGANDO BIEN
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
			pthread_mutex_lock (&mutexMem);
			// Lo muestro aca porque si lo muestro despues de que lo mate, la tabla no tiene el registro
			mostrarVersus(tablaAccesos, registro_prueba->PID);
			matarProceso(registro_prueba, tabla_adm, TLB, tablaAccesos);
			envioAlSwap(registro_prueba, serverSocket, NULL, flag );

			if(flag)
			{
				log_info(logger, "Se hizo conexion con swap, se envio proceso a matar y este fue recibido correctamente");
				bool recibi = true;
				send(socketCliente,&recibi,sizeof(bool),0);

				puts("Paaso");
			}else
			{
				bool recibi = false;
				send(socketCliente,&recibi,sizeof(bool),0);
				log_error(logger, "Hubo un problema con la conexion/envio al swap");
			}

			pthread_mutex_unlock (&mutexMem);
	 		break;
	 	default:
			printf ("El tipo de ejecucion recibido no es valido\n");
	 		break;
	 	}
	free(flag);
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
			list_add(lista_proceso,pag_proc_create(x, NULL, -1, 0 , 0));
			//cuando la creo el marco lo pongo en -1
			x++;
		}

		list_add(tabla_adm,tabla_adm_create(proceso->PID, lista_proceso) );
}

int leerDesdeTlb(int socketCliente, t_list * TLB, int pid, int pagina, t_list* tablaAccesos)
{
	bool _numeroDePid (void * p)
	{
			return(*(int *)p == pid);
	}
	bool _numeroDePagina (void * p)
	{
			return(*(int *)p == pagina);
	}
	int * posicion = malloc(sizeof(int));
	t_header * pac = package_create(0, pid, pagina, 0);
	t_tlb * registro_tlb = buscarEntradaProcesoEnTlb(TLB, pac, posicion );
	// SI LA ENCONTRO LA LEO Y SE LA ENVIO AL CPU
	if (registro_tlb != NULL)
	{
		upPaginasAccedidas(tablaAccesos, registro_tlb->pid);
		log_info(logger, " Encontre la pagina para leer en la tlb y dice -> %s", registro_tlb->direccion_fisica);
		bool recibi = true;
		send(socketCliente,&recibi,sizeof(bool),0);
		//send(socketCliente,registro_tlb->direccion_fisica,miContexto.tamanioMarco,0);
		return 1;
	}
	// SI LA TLB NO ESTA HABILITADA ENTONCES TENGO QUE VERIFICAR EN LA TABLA DE TABLAS
	//  SI YA ESTA CARGADA EN MEMORIA O SI ESTA EN SWAP
	return 0;
}

int leerEnMemReal(t_list * tabla_adm, t_list * TLB, t_header * package, int serverSocket, int socketCliente, char * memoria_real, t_list* tablaAccesos)
{
	bool _numeroDePid (void * p)
	{
		return(*(int *)p == package->PID);
	}

	int  * flag = malloc(sizeof(int));

	// SLEEP PORQUE LA MEMORIA BUSCA EN SUS ESTRUCTURAS
	sleep(miContexto.retardoMemoria);
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
		process_pag * pagina_proc = list_find(tabla_proc, (void *)_numeroDePagina);

		// SI LA DIRECCION ES NULL ES PORQUE ESTA EN SWAP, SINO YA LA ENCONTRE EN MEMORIA
		if ( pagina_proc->direccion_fisica == NULL)
		{
			log_info(logger, "Enontre la pagina para leer en swap");
			if ( marcosProcesoLlenos(tabla_proc))
			{
				int verific = swapeando(tabla_proc,tabla_adm ,TLB, NULL, serverSocket, package, tablaAccesos);
				if (verific != 1)
				{
					log_error(logger, "Error al intentar swapear");
					puts("No Verifico");
					bool recibi = false;
					send(socketCliente,&recibi,sizeof(bool),0);
				}
				else
				{
					upPaginasAccedidas(tablaAccesos, package->PID);
					upFallosPagina(tablaAccesos, package->PID);
					bool recibi = true;
					send(socketCliente,&recibi,sizeof(bool),0);
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
					envioAlSwap(package, serverSocket, contenido, flag);
					//SI TODO SALIO BIEN, EL SWAP CARGO LA PAGINA A LEER EN "CONTENIDO"
					if(*flag)
					{
						t_marco_hueco * marco_a_llenar = list_remove(listaFramesHuecosMemR, 0);
						log_info(logger, "Traje la pagina del swap, voy a escribir el marco %d para leer", marco_a_llenar->numero_marco);
						sleep(miContexto.retardoMemoria);
						// LO ESCRIBO CON EL MENSAJE QUE ME DICEN QUE LO ESCRIBA PORQUE NO TENGO QUE TRAER LO QUE YA ESTE ESCRITO DEL SWAP
						memcpy ( marco_a_llenar->direccion_inicio, contenido, strlen(contenido));
						//AGREGO EL MARCO AHORA ESCRITO, A LA LISTA DE MARCOS ESCRITOS
						list_add(listaFramesMemR, marco_a_llenar);

						//AGREGO LA PAGINA A LA TLB (VERIFICO SI ESTA LLENA Y REEMPLAZO)
						pthread_mutex_lock (&mutexTLB);
						if (!strcmp(miContexto.tlbHabilitada, "SI"))
						actualizarTlb(package->PID, package->pagina_proceso, marco_a_llenar->direccion_inicio, TLB, marco_a_llenar->numero_marco);
						pthread_mutex_unlock (&mutexTLB);
						// ACTUALIZO LA TABLA DEL PROCESO CON LA DRIECCION FISICA, DEPENDIENDO EL ALGORITMO DEL CONTEXTO
						actualizoTablaProceso(tabla_proc, marco_a_llenar, package);
						upPaginasAccedidas(tablaAccesos, package->PID);

						log_info(logger, "Se hizo conexion con swap, se envio paquete a leer y este fue recibido correctamente");
						lectura(package, tabla_adm, memoria_real, contenido, TLB, pagina_proc);
						// Como la transferencia con el swap fue exitosa, le envio la pagina al CPU
						bool recibi = true;
						send(socketCliente,&recibi,sizeof(bool),0);
						upFallosPagina(tablaAccesos, package->PID);

						return 1;
					}
					else
					{
						bool recibi= false;
						send(socketCliente,&recibi,sizeof(bool),0);
						log_error(logger, "Hubo un problema con la conexion/envio al swap");
						return 0;
					}
				}else
				{
					log_info(logger, "Ya no tengo mas marcos disponibles en la memoria, rechazo pedido");
					mostrarVersus(tablaAccesos, package->PID);
					matarProceso(package, tabla_adm, TLB, tablaAccesos);
				}

			}
		}else // SI NO ESTA EN SWAP, YA CONOZCO LA DIRECCION DE SU MARCO //
		{
			printf("LEI PORQUE ESTABA EN MEMORIA -> %s Y MANDO A ACTUALIZAR TABLAS \n", pagina_proc->direccion_fisica);
			if(!strcmp(miContexto.algoritmoReemplazo, "LRU"))
			{
				actualizarTablaProcesoLru(tabla_proc, package->pagina_proceso, pagina_proc->direccion_fisica, pagina_proc->marco);

			}
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
					actualizarTlb(package->PID, package->pagina_proceso, pagina_proc->direccion_fisica, TLB, pagina_proc->marco);
		 	upPaginasAccedidas(tablaAccesos, package->PID );
		 	bool recibi = true;
		 	send(socketCliente,&recibi,sizeof(bool),0);
			//send(socketCliente,pagina_proc->direccion_fisica,sizeof(pagina_proc->direccion_fisica),0);
			return 1;
		}
	}else
	{
		log_info(logger, "Se esta queriendo leer una pagina de un proceso que no esta iniciado");
	}
	return 0;
}

t_list * obtenerTablaProceso(t_list * tabla_adm, int pid)
{
	bool _numeroDePid (void * p)
	{
		return(*(int *)p == pid);
	}
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
	bool _numeroDePagina (void * p)
	{
		return(*(int*)p == pagina);
	}
 	 // TRAIGO LA PAGINA BUSCADA
	process_pag * pagina_proc = list_find(tabla_proceso, (void *)_numeroDePagina);
	return pagina_proc;
 }

/*
 * KOLO
 * No pongo semaforos porque unicamente se la llama en leerEnMemReal que ya tiene el sem
 */
void lectura(t_header * proceso_entrante, t_list * tabla_adm, char * memoria_real, char * contenido, t_list * TLB, process_pag * pagina_proceso)
{
	// TRAIGO EL PRIMER MARCO VACIO DE MI MEMORIA PARA ALMACENAR EL CONTENIDO A LEER
	t_marco_hueco * marco_vacio = (t_marco_hueco*)listaFramesHuecosMemR->head;

	pagina_proceso->pag = proceso_entrante->pagina_proceso;
	pagina_proceso->direccion_fisica = marco_vacio->direccion_inicio;

	// PASO EL MARCO LIBRE A LA LISTA DE OCUPADOS
	t_marco * marco_ocupado = marco_create(marco_vacio->direccion_inicio, marco_vacio->numero_marco);
	list_add(listaFramesMemR, marco_ocupado);

	// ESCRIBO EN EL MARCO LA PAGINA QUE RECIBI DEL SWAP (ES PARA LECTURA)
	strcpy(pagina_proceso->direccion_fisica, contenido);

	// LIBERO EL NODO/MARCO DE LA LISTA DE MARCOS HUECOS PORQUE AHORA ESTA EN LA DE OCUPADOS.
	list_remove(listaFramesHuecosMemR, 0);

	// SI LA TLB ESTA HABILITADA Y NO ESTA LLENA, ENTONCES LE CREO LA ENTRADA DE LA PAGINA LEIDA
	pthread_mutex_lock (&mutexTLB);
	if( !strcmp(miContexto.tlbHabilitada, "SI") && !tlbLlena(TLB))
	 	actualizarTlb(proceso_entrante->PID, proceso_entrante->pagina_proceso, pagina_proceso->direccion_fisica, TLB, pagina_proceso->marco);
	pthread_mutex_unlock (&mutexTLB);
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
	int numero_de_pid = proceso_entrante->PID;

	bool _numeroDePid (void * p)
	{
		return(*(int *)p == numero_de_pid);
	}
	t_tabla_adm * registro_tabla_proc = list_find(tabla_adm, (void*)_numeroDePid);

	list_remove_by_condition(tablaAccesos, (void*)_numeroDePid);

	if (registro_tabla_proc != NULL)
	{
		t_list * tabla_proceso = registro_tabla_proc->direc_tabla_proc;

		if (tabla_proceso != NULL)
		{
			int cantidad_paginas = tabla_proceso->elements_count;
			int x = 0;
			while (x < cantidad_paginas)
			{
				process_pag * pagina_removida = list_remove(tabla_proceso, 0);

				bool _numMarco (void * p)
				{
					return(*(char *)p == pagina_removida->marco);
				}

				if ( pagina_removida->direccion_fisica == NULL)
				{

				}
				else{
					//ENCONTRO PAGINA A REMOVER QUE TENIA UN MARCO ASIGNADO
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
			pthread_mutex_lock (&mutexTLB);
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
				removerEntradasTlb(TLB, proceso_entrante);
			pthread_mutex_unlock (&mutexTLB);
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
		list_remove_and_destroy_by_condition(TLB,(void*)_numeroDePid,(void*)reg_tlb_destroy);
		printf("ELEMENTOS DE LA TLB DESPUES DE MATAR -> %d \n", TLB->elements_count);
	}
	return 1;
}

int verificarTlb (t_list * TLB, int tamanio_msg, char * message, t_header * pagina, t_list* tablaAccesos)
{
	int * posicion = malloc(sizeof(int));
	t_tlb * registro_tlb = buscarEntradaProcesoEnTlb(TLB, pagina, posicion);
	//Actualizo cantidad de accesos a TLB;
	cantAccesosTlb=cantAccesosTlb+1;
	free(posicion);
	if (registro_tlb != NULL)
	{
		upPaginasAccedidas(tablaAccesos, pagina->PID);
		// No me reconoce el argumento "marco" de la estructura tlb y los demas si, es una joda esto
		log_info(logger, "TLB HIT pagina: %d en el marco numero: %d", registro_tlb->pagina/*registro_tlb->marco*/);
		//Actualizo cantidad de aciertos
		cantHitTlb= cantHitTlb +1;
		strcpy (registro_tlb->direccion_fisica, message);
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

// ARREGLAR ESTA FUNCION, ESTA TODO EL CODIGO DUPLICADO ESCRITURA/LECTURA
/*
 * KOLO
 * No pongo semaforo porque la llama una sola vez, y ya tiene el sem ahi
 */
int swapeando(t_list* tablaProceso,t_list* tabla_adm , t_list * TLB, char * mensaje, int serverSocket, t_header * header, t_list* tablaAccesos)
{
	// TRAIGO LA PRIMER PAGINA QUE SE HAYA CARGADO EN MEMORIA
	process_pag * paginaASwapear = traerPaginaARemover(tablaProceso);
	log_info(logger, "Acceso a swap: Voy a swapear para traer la pagina %d porque no tengo lugar para este proceso", header->pagina_proceso);
	printf("Pagina a remover: %d \n", paginaASwapear->pag); // PRINT SOLO PARA VERIFICAR, DESPUÉS BORRAR
	sleep(miContexto.retardoMemoria);

	int num_pag;

	bool _numeroDePag (void * p)
	{
		return(*(int*)p == num_pag);
	}

	// SI SE TRATA DE UNA ESCRITURA
	if (header->type_ejecution == 1)
	{
		t_header * header_escritura = crearHeaderEscritura( header->PID, paginaASwapear->pag, strlen(mensaje));

		int * status_escritura = malloc(sizeof(int));
		envioAlSwap(header_escritura, serverSocket, paginaASwapear->direccion_fisica, status_escritura);

		if ( *status_escritura != 1)
		{
			log_error(logger, "No se pudo escribir en swap");
			return 0;
		}

		// LE PIDO LA PAGINA QUE QUIERO ESCRIBIR Y LA AGREGO AL FINAL DE LA LISTA
		t_header * header_lectura = crearHeaderLectura(header);

		int * status_lectura = malloc(sizeof(int));
		char * contenido = malloc(miContexto.tamanioMarco);
		envioAlSwap(header_lectura, serverSocket, contenido, status_lectura);

		num_pag = header->pagina_proceso;


		if (*status_lectura != 1)
		{
			log_error(logger, "No se pudo leer de swap");
			return 0;
		}

		// Escribo en mi pagina swapeada el contenido a escribir
		log_info(logger, "Escribo el marco de mi pagina swapeada para escribir");
		sleep(miContexto.retardoMemoria);
		strcpy(paginaASwapear->direccion_fisica, mensaje );

		num_pag = paginaASwapear->pag;
		free(contenido);
		free(status_escritura);
		free(status_lectura);
		free(header_escritura);
		free(header_lectura);
		// SI SE TRATA DE UNA LECTURA
	}else if(header->type_ejecution ==0)
	{
		int * status_lectura = malloc(sizeof(int));
		char * contenido = malloc(sizeof(miContexto.tamanioMarco));
		envioAlSwap(header, serverSocket, contenido, status_lectura);

		if ( *status_lectura != 1)
		{
			log_error(logger, "No se pudo leer de swap");
			return 0;
		}

		log_info(logger, "Escribo el marco de mi pagina swapeada para leer");
		sleep(miContexto.retardoMemoria);
		strcpy(paginaASwapear->direccion_fisica, contenido );

		num_pag = paginaASwapear->pag;
	}

	//ACTUALIZO LA TABLA DEL PROCESO SEGUN ALGORITMOS // NO USO ACTUALIZOTABLAPROCESO POR LOS PARAMETROS
	if ( !strcmp(miContexto.algoritmoReemplazo, "FIFO"))
	{
		printf("SWAPEA CORTE FIFO \n");
		actualizarTablaProcesoFifo(tablaProceso, header->pagina_proceso, paginaASwapear->direccion_fisica, paginaASwapear->marco);
	}else if (!strcmp(miContexto.algoritmoReemplazo, "LRU")) // LA MUE
	{
		printf("SWAPEA CORTE LRU \n");
		actualizarTablaProcesoLru(tablaProceso, header->pagina_proceso, paginaASwapear->direccion_fisica, paginaASwapear->marco);
	}else // CLOCK POR DESCARTE YA QUE NO PUEDE HABER ERRORES EN EL ARCHIVO DE CONFIGURACION, ACTUALIZA EL BIT DE ACCEDIDO A 1
	{
		printf("SWAPEA CORTE CLOCK \n");
		actualizarTablaProcesoClock(tablaProceso, header->pagina_proceso, paginaASwapear->direccion_fisica, paginaASwapear->marco);
	}

	pthread_mutex_lock (&mutexTLB);
	if (!strcmp(miContexto.tlbHabilitada, "SI"))
	{
		int * posicion = malloc(sizeof(int));
		t_header * asd =  package_create(0, header->PID, paginaASwapear->pag, 0);
		t_tlb * tlb_reg = buscarEntradaProcesoEnTlb(TLB, asd ,posicion);

		// SI LA ENCONTRO EN LA TLB
		if (*posicion != -1)
		{
			log_info(logger, "Borro la entrada de la TLB de la posicion-->%d", *posicion);
			list_remove(TLB, *posicion);
		}
		actualizarTlb(header->PID, header->pagina_proceso, paginaASwapear->direccion_fisica, TLB, paginaASwapear->marco);
	}
	pthread_mutex_unlock (&mutexTLB);

	// ACTUALIZO LA PAGINA QUE SWAPEE ( LA ELIMINO Y LA VUELVO A AGREGAR VACIA DEL TODO )
	list_remove_by_condition(tablaProceso, (void*)_numeroDePag);
	list_add(tablaProceso, pag_proc_create(paginaASwapear->pag, NULL, -1, 0, 0));

	//upPaginasAccedidas(tablaAccesos, header->PID);
	//upFallosPagina(tablaAccesos, header->PID);

	return 1;
}

void actualizoTablaProceso(t_list * tablaProceso, t_marco_hueco * marco_a_llenar , t_header * registro_prueba)
{
	if(!strcmp(miContexto.algoritmoReemplazo, "FIFO"))
	{
		printf("Entra a actualizar corte FIFO \n");
		if(marco_a_llenar!=NULL)
			actualizarTablaProcesoFifo(tablaProceso, registro_prueba->pagina_proceso, marco_a_llenar->direccion_inicio, marco_a_llenar->numero_marco);
		else
			actualizarTablaProcesoFifo(tablaProceso, registro_prueba->pagina_proceso, NULL, NULL);
	}else if (!strcmp(miContexto.algoritmoReemplazo, "LRU"))
	{
		printf("Entra a actualizar corte LRU \n");
		if(marco_a_llenar!=NULL)
			actualizarTablaProcesoLru(tablaProceso, registro_prueba->pagina_proceso, marco_a_llenar->direccion_inicio, marco_a_llenar->numero_marco);
		else
			actualizarTablaProcesoLru(tablaProceso, registro_prueba->pagina_proceso, NULL, NULL);
	}else
	{
		printf("Entra a actualizar corte LRU \n");
		if(marco_a_llenar!=NULL)
			actualizarTablaProcesoClock(tablaProceso, registro_prueba->pagina_proceso, marco_a_llenar->direccion_inicio, marco_a_llenar->numero_marco);
		else
			actualizarTablaProcesoClock(tablaProceso, registro_prueba->pagina_proceso, NULL, NULL);
	}
}

void actualizarTablaProcesoLru(t_list * tabla_proceso, int num_pagina, char * direccion_marco, int num_marco)
{
	bool _numeroDePagina (void * p)
	{
		return(*(int*)p == num_pagina);
	}

	process_pag * pag = list_remove_by_condition(tabla_proceso, (void*)_numeroDePagina);
	process_pag * pagina;
	if(direccion_marco!=NULL)
		pagina = pag_proc_create(num_pagina, direccion_marco, num_marco, 0, 0);
	else
		pagina = pag_proc_create(num_pagina, pag->direccion_fisica, pag->marco, 0, 0);
	list_add(tabla_proceso,pagina);
	puts("Final actualizarTablaProcesoLru");
}

void actualizarTablaProcesoFifo(t_list * tabla_proceso, int num_pagina, char * direccion_marco, int num_marco)
{
	bool _numeroDePagina (void * p)
	{
		return(*(int*)p == num_pagina);
	}

	process_pag * pagina = list_find(tabla_proceso, (void*)_numeroDePagina);
	if( pagina->direccion_fisica == NULL)
	{
		list_remove_by_condition(tabla_proceso, (void*)_numeroDePagina);
		pagina->direccion_fisica = direccion_marco;
		pagina->marco = num_marco;
		list_add(tabla_proceso, pagina);
	}else
	{
		// TECNICAMENTE SI YA ESTABA CARGADA, LA DEJO EN LA POSICION EN LA QUE ESTABA
	}

}

void actualizarTablaProcesoClock(t_list * tabla_proceso, int num_pagina, char * direccion_marco, int num_marco)
{
	bool _numeroDePagina (void * p)
	{
			return(*(int *)p == num_pagina);
	}

	process_pag * pagina = list_find(tabla_proceso, (void*)_numeroDePagina);
	pagina->direccion_fisica = direccion_marco;
	pagina->marco = num_marco;
	pagina->accessed = 1;
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
			return ambosBitsEnCero(tablaProceso);
		}else
		{
			return pagina;
		}
	// SI ENCONTRE UNA PAGINA CON AMBOS BITS EN CERO, LA RETORNO
	}else
	{
		return pagina;
	}
}

process_pag * ambosBitsEnCero(t_list * tablaProceso)
{
	int tamanio = tablaProceso->elements_count;
	int x = 0;
	while ( x < tamanio)
		{
			process_pag * pagina = list_get(tablaProceso, x);
			if ( pagina->accessed == 0 && pagina->dirty == 0)
			{
				return pagina;
			}
			x++;
		}
	return NULL;
}

process_pag * bitDirtyEnUno (t_list * tablaProceso)
{
	int y = 0;
	int tamanio = tablaProceso->elements_count;
	while ( y < tamanio)
	{
		process_pag * pagina = list_get(tablaProceso, y);
		if( pagina->accessed == 0 && pagina->dirty == 1)
		{
			return pagina;
		}else if ( ( pagina->accessed == 1 && pagina->dirty == 0) && ( pagina->accessed == 1 && pagina->dirty == 1) )
		{
			pagina->accessed = 0;
		}
		y++;
	}
	return NULL;
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
	printf("Paginas ocupadas: %d \n",paginas_ocupadas);
	printf("Paginas disponibles: %d \n",paginas_disponibles);
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
			puts("Paso el add");
		}
	}
}

void upPaginasAccedidas(t_list* tablaAccesos, int pid)
{
	bool _numeroDePid (void * p)
	{
			return(*(int *)p == pid);
	}

	t_versus * reg = list_find(tablaAccesos, (void*)_numeroDePid);
	reg->cantPagAccessed++;
}

void upFallosPagina(t_list* tablaAccesos, int pid)
{
	bool _numeroDePid (void * p)
	{
			return(*(int *)p == pid);
	}

	t_versus * reg = list_find(tablaAccesos, (void*)_numeroDePid);
	reg->cantFallosPag++;

	printf("KKKKKKKKKKKKKKKKKK AHORA VAN %d FALLOS", reg->cantFallosPag);

}


void mostrarVersus(t_list* tablaAccesos, int pid)
{
	bool _numeroDePid (void * p)
	{
			return(*(int *)p == pid);
	}

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

int procesoTienePaginaCargada(t_list* tabla_proceso)
{
	int r = 0;
	int cant_pag = tabla_proceso->elements_count;
	int x = 0;
	while (x < cant_pag)
	{
		process_pag * pagina = list_get(tabla_proceso, x);
		if ( pagina->direccion_fisica != NULL)
		{
			r++;
		}
		x++;
	}
	return r;
}

void removerMarcoPorMarco(int marco)
{
	int cant_marcos = listaFramesMemR->elements_count;
	int x = 0;

	while ( x < cant_marcos)
	{
		t_marco_hueco * marco_reg = list_get(listaFramesMemR, 0);
		if ( marco_reg->numero_marco == marco)
		{
			t_marco_hueco * marco_a_remover = list_remove(listaFramesMemR, x);

			//AGREGO EL MARCO AHORA HUECO, A LA LISTA DE MARCOS HUECOS
			// SIGUE TENIENDO SU DIRECCION Y SU NUMERO DE MARCO, NO IMPORTA EN QUE LISTA ESTE
			list_add(listaFramesHuecosMemR, marco_a_remover);
		}
		x++;
	}
}

//------SEÑALES QUE TIENE QUE RECIBIR LA MEMORIA-------------//
/*
 * Cuando la memoria recibe esta señal, debe limpiar la TLB.
 */
void tlbFlush(t_list * TLB)
{
	pthread_mutex_lock (&mutexTLB);
	if (!strcmp(miContexto.tlbHabilitada,"SI"))
	{
		int i=0;
		int cant_elementos = TLB->elements_count;
		for(;i<cant_elementos;i++)
		{
			list_remove_and_destroy_element(TLB, 0,(void*)reg_tlb_destroy);
		}
	}
	else
	{
		puts("La TLB NO esta habilitada. \n");
	}
	pthread_mutex_unlock (&mutexTLB);
}

/*
 * Cuando se recibe esta señal, se debe limpiar completamente la memoria principal,
 * actualizando los bits que sean necesarios en las tablas de páginas de los diferentes procesos
 */
void limpiarMemoria(void * args)
{
	pthread_mutex_lock (&mutexMem);
	parametros * param;
	param = (parametros * ) args;
	puts("Recibi el lock");

	//Vacio la memoria
	int k = 0;
	char * punteroAMemoria=  param->memoria;
	while(k<miContexto.cantidadMarcos*miContexto.tamanioMarco)
	{
		strcpy(punteroAMemoria,"");
		punteroAMemoria++;
		k++;
	}
	free(punteroAMemoria);


	//Actualizo las listas
	int i = 0, j = 0;
	for(;i<param->tabla_adm->elements_count;i++) //Recorro la tabla de tablas
	{
		puts("Actualizo listas");
		//Traigo una tabla
		t_tabla_adm * entrada_tabla_tablas = list_get(param->tabla_adm,i);
		t_list * tablaProceso = entrada_tabla_tablas->direc_tabla_proc;

		for(;j<tablaProceso->elements_count;j++) //Recorro la tabla de procesos
		{
			process_pag * pagina_proc = list_get(tablaProceso, j); //Traigo una pagina
			//Actualizo la pagina
			pagina_proc->direccion_fisica=NULL;
			pagina_proc->marco=-1;
			pagina_proc->dirty=0;
			pagina_proc->accessed=0;
		}
	}

	//Actualizo marcos
	list_destroy_and_destroy_elements(listaFramesMemR,(void *)marco_destroy);
	list_destroy_and_destroy_elements(listaFramesHuecosMemR,(void *)marco_hueco_destroy);
	listaFramesMemR = crearListaFrames();
	listaFramesHuecosMemR = crearListaHuecosFrames(miContexto.cantidadMarcos, miContexto.tamanioMarco, param->memoria);
	tlbFlush(param->tlb);
	pthread_mutex_unlock (&mutexMem);
}
/*
 * Cuando recibe esta señal se deberá realizar un volcado (dump) del contenido de la memoria principal,
 * en el archivo log de Administrador de Memoria, creando para tal fin un proceso nuevo.
 * Se recomienda usar fork().
 */

void dumpEnLog(char * memoria_real, t_list * tablaAdm)
{
	pthread_mutex_lock (&mutexMem);
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
				log_info(logger,"Marco: %d ; Contenido: %s",pagina_proc->marco, pagina_proc->direccion_fisica);
			}
		}
	}
	pthread_mutex_unlock (&mutexMem);

}

void tasasDeTLB()
{
	int porcentajeHit = (cantHitTlb * 100)/cantAccesosTlb;
	log_info(logger, "La cantidad de accesos a la TLB es: %d \n", cantAccesosTlb);
	log_info(logger, "La cantidad de aciertos de la TLB es: %d \n", cantHitTlb);
	log_info(logger, "La tasa de aciertos de la TLB es: %d%\n", porcentajeHit);
}
//-----------------------------------------------------------//

// --------------ENTRADAS A LA TABLA DE PROCESO ------------ //
process_pag * pag_proc_create (int pagina, char * direccion_fisica, int marco, int accedido, int dirty)
{
 process_pag * reg_pagina = malloc(sizeof(process_pag));
 reg_pagina->pag = pagina;
 reg_pagina->direccion_fisica = direccion_fisica;
 reg_pagina->marco = marco;
 reg_pagina->accessed = accedido;
 reg_pagina->dirty = dirty;
 return reg_pagina;
}

void pag_proc_destroy(process_pag * self)
{
	free(self);
}
// --------------------------------------------------------//

