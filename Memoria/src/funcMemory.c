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

			// PRIMERO VERIFICO QUE LA TLB ESTE HABILITADA*/
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
			{
				pthread_mutex_lock (&mutexTLB);
				flag=leerDesdeTlb(socketCliente,TLB, registro_prueba->PID, registro_prueba->pagina_proceso);
				pthread_mutex_unlock (&mutexTLB);
				if(!flag)
				{
					pthread_mutex_lock (&mutexMem);
					leerEnMemReal(tabla_adm,TLB, registro_prueba, serverSocket,socketCliente, memoria_real);
					pthread_mutex_unlock (&mutexMem);
				}
			}
			else
			{
				pthread_mutex_lock (&mutexMem);
				leerEnMemReal(tabla_adm, TLB,registro_prueba, serverSocket,socketCliente, memoria_real);
				pthread_mutex_unlock (&mutexMem);
			}
	 		break;

	 	case 1:
			//printf ("*********************Se recibio orden de escritura*********************\n");
			log_info(logger, "Solicitud de escritura recibida del PID: %d y pagina: %d", registro_prueba->PID, registro_prueba->pagina_proceso);
			int okTlb;
			// DECLARO UN FLAG PARA SABER SI ESTABA EN LA TLB Y SE ESCRIBIO, O SI NO ESTABA
			pthread_mutex_lock (&mutexMem);
			if (!strcmp(miContexto.tlbHabilitada, "SI"))
			{
				okTlb = verificarTlb(TLB,registro_prueba->tamanio_msj, mensaje, registro_prueba);
			}
			else
			{
				okTlb = 0;
			}

			pthread_mutex_unlock (&mutexMem);

			// SI ESTABA EN LA TLB, YA LA FUNCION ESCRIBIO Y LISTO
			if(okTlb == 1)
			{
				pthread_mutex_lock (&mutexMem);
				// TRAIGO LA TABLA DEL PROCESO
				t_list * tablaProceso = obtenerTablaProceso(tabla_adm, registro_prueba->PID);
				// TRAIGO LA PAGINA A ESCRIBIR ( SOLO EL NODO DE LA TABLA DEL PROCESO )
				process_pag * paginaProceso = obtenerPaginaProceso(tablaProceso, registro_prueba->pagina_proceso);
				pthread_mutex_unlock (&mutexMem);

				// SE ESCRIBIO CORRECTAMENTE PORQUE YA ESTABA CARGADA EN TLB
				pthread_mutex_lock (&mutexTLB);
				if(!strcmp(miContexto.tlbHabilitada, "SI"))
				{
					actualizarTlb(registro_prueba->PID, registro_prueba->pagina_proceso, paginaProceso->direccion_fisica, TLB);
				}
				pthread_mutex_unlock (&mutexTLB);
				printf ("SE ESCRIBIO CORRECTAMENTE PORQUE ESTABA EN LA TLB \n");
			}
			// SI NO ESTABA EN LA TLB, AHORA ME FIJO SI ESTA EN LA TABLA DE TABLAS
			else
			{
				pthread_mutex_lock (&mutexMem);
				// TRAIGO LA TABLA DEL PROCESO
				t_list * tablaProceso = obtenerTablaProceso(tabla_adm, registro_prueba->PID);

				// TRAIGO LA PAGINA A ESCRIBIR ( SOLO EL NODO DE LA TABLA DEL PROCESO )
				process_pag * paginaProceso = obtenerPaginaProceso(tablaProceso, registro_prueba->pagina_proceso);

				// SI ESTA EN SWAP
				if (!strcmp(paginaProceso->direccion_fisica,"Swap"))
				{
					/* SI NO TENGO ESPACIO PARA TRAERLA (TODOS LOS MARCOS DISPONIBLES PARA ESE
					 * PROCESO YA ESTAN LLENOS), SWAPEO LA PRIMER PAGINA CARGADA (FIFO)
					 * Y ESCRIBO LA QUE RECIBO DEL SWAP AL FINAL DE LA LISTA
					 */
					if ( marcosProcesoLlenos(tablaProceso))
					{
						//printf("NO TENGO LUGAR PARA GUARDARLA, TENGO QUE SWAPEAR \n");
						int verific = swapeando(tablaProceso,tabla_adm ,TLB, mensaje, serverSocket, registro_prueba);
						if (verific == 1)
						{	pthread_mutex_lock (&mutexTLB);
							if(!strcmp(miContexto.tlbHabilitada, "SI"))
							{
								actualizarTlb(registro_prueba->PID, registro_prueba->pagina_proceso, paginaProceso->direccion_fisica, TLB);
							}
							pthread_mutex_unlock (&mutexTLB);
						}
						else
							log_error(logger, "Error al intentar swapear");
					}
					/* SI TENGO ESPACIO PARA TRAERLA (CANT MAX DE MARCOS PARA ESE PROCESO
					 *NO FUE ALCANZADA TODAVÍA), SI ME QUEDA MEMORIA (MARCOS) LA TRAIGO Y LA ESCRIBO
					 */
					else
					{
						// Creo el header de lectura de una pagina y se lo mando al swap
						t_header * lectura_swap = crearHeaderLectura(registro_prueba);
						/* char para guardar el contenido de la pagina que tengo que traer del swap para escribir
						 * (Entendiendo que tengo que traer el contenido y escribir a continuacion)
						 */
						char * contenido_a_escribir = malloc(miContexto.tamanioMarco);

						int status_lectura2 = envioAlSwap(lectura_swap, serverSocket, contenido_a_escribir);

						if (status_lectura2 == 1)
						{
							// TENGO QUE ASIGNARLE UNA DIRECCION PARA ESCRIBIR AHI, TRAIGO UN MARCO HUECO Y ESCRIBO
							if (listaFramesHuecosMemR->elements_count != 0)
							{
								t_marco_hueco * marco_a_llenar = list_remove(listaFramesHuecosMemR, 0);
								//marco_a_llenar->direccion_inicio=malloc(miContexto.tamanioMarco);
								log_info(logger, "Traje la pagina del swap, voy a escribir el marco %d", marco_a_llenar->numero_marco);
								sleep(miContexto.retardoMemoria);
								memcpy ( marco_a_llenar->direccion_inicio, mensaje, strlen(mensaje));

								//AGREGO EL MARCO AHORA ESCRITO, A LA LISTA DE MARCOS ESCRITOS
								list_add(listaFramesMemR, marco_a_llenar);

								//AGREGO LA PAGINA A LA TLB (VERIFICO SI ESTA LLENA Y REEMPLAZO)
								pthread_mutex_lock (&mutexTLB);
								if(!strcmp(miContexto.tlbHabilitada, "SI"))
								{
									actualizarTlb(registro_prueba->PID, registro_prueba->pagina_proceso, marco_a_llenar->direccion_inicio, TLB);
								}
								pthread_mutex_unlock (&mutexTLB);
								// ACTUALIZO LA TABLA DEL PROCESO CON LA DRIECCION FISICA
								actualizarTablaProceso(tablaProceso, registro_prueba->pagina_proceso, marco_a_llenar->direccion_inicio, marco_a_llenar->numero_marco);
							}else
							{
								/*  KOLO PREGUNTAR SI HAY QUE HACER ESTO O AUNQUE EL PROCESO TENGA ESPACIO
								 * SWAPEAR PORQUE NO HAY MARCOS EXTRAS
								 * Y QUE PASARIA SI NO HAY NINGUNA PAGINA DE ESE PROCESO CARGADA
								 */
								log_info(logger, "Ya no tengo mas marcos disponibles en la memoria, rechazo pedido");
							}
						}
					}
					// SI NO ESTA EN SWAP, ENTONCES okMem TIENE LA DIRECCION DEL MARCO PARA ESCRIBIR EL MENSAJE
				}else
				{
					log_info(logger, "Encontre la pagina en memoria, la escribo y acualizo tlb");
					sleep(miContexto.retardoMemoria);
					memcpy ( paginaProceso->direccion_fisica, mensaje, registro_prueba->tamanio_msj);
					pthread_mutex_lock (&mutexTLB);
					if(!strcmp(miContexto.tlbHabilitada, "SI"))
					{
						actualizarTlb(registro_prueba->PID, registro_prueba->pagina_proceso, paginaProceso->direccion_fisica, TLB);
					}
					pthread_mutex_unlock (&mutexTLB);
				}
				pthread_mutex_unlock (&mutexMem);
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
	 			pthread_mutex_lock (&mutexMem);
	 			iniciarProceso(tabla_adm, registro_prueba);
	 			pthread_mutex_unlock (&mutexMem);
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
			pthread_mutex_lock (&mutexMem);
			matarProceso(registro_prueba, tabla_adm, TLB);
			pthread_mutex_unlock (&mutexMem);
			int flag = envioAlSwap(registro_prueba, serverSocket, NULL );

			if(flag)
				log_info(logger, "Se hizo conexion con swap, se envio proceso a matar y este fue recibido correctamente");
			else
				log_error(logger, "Hubo un problema con la conexion/envio al swap");
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
			list_add(lista_proceso,pag_proc_create(x, "Swap" , -1));
			//cuando la creo el marco lo pongo en -1
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
	if (subListaProceso->elements_count != 0)
	{
		// VERIFICO QUE ALGUNA DE LAS ENTRADAS TENGA LA PAGINA QUE BUSCO
		t_tlb * registro_tlb = list_find(subListaProceso, (void *)_numeroDePagina);

		// SI LA ENCONTRO LA LEO Y SE LA ENVIO AL CPU
		if (registro_tlb != NULL)
		{
			send(socketCliente,registro_tlb->direccion_fisica,miContexto.tamanioMarco,0);
			return 1;
		}
	// SI LA TLB NO ESTA HABILITADA ENTONCES TENGO QUE VERIFICAR EN LA TABLA DE TABLAS
	//  SI YA ESTA CARGADA EN MEMORIA O SI ESTA EN SWAP
	}
	return 0;
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
	if (reg_tabla_tablas!=NULL)
	{
		// TRAIGO LA TABLA DEL PROCESO
		t_list * tabla_proc = reg_tabla_tablas->direc_tabla_proc;

		// TRAIGO LA PAGINA BUSCADA
		process_pag * pagina_proc = list_find(tabla_proc, (void *)numeroDePaginaIgualA);

		// SI LA DIRECCION CONTIENE = "Swap" ES PORQUE ESTA EN SWAP, SINO YA LA ENCONTRE EN MEMORIA
		if (!strcmp(pagina_proc->direccion_fisica,"Swap"))
		{
			char * contenido = malloc(miContexto.tamanioMarco);
			flag = envioAlSwap(package, serverSocket, contenido );
			//SI TODO SALIO BIEN, EL SWAP CARGO LA PAGINA A LEER EN "CONTENIDO"
			if(flag)
			{
				log_info(logger, "Se hizo conexion con swap, se envio paquete a leer y este fue recibido correctamente");
				lectura(package, tabla_adm, memoria_real, contenido, TLB, pagina_proc);
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
	t_tabla_adm * reg_tabla_tablas = list_find(tabla_adm, (void*)_numeroDePid);

	// SI ENCONTRO UN REGISTRO CON ESE PID
	if (reg_tabla_tablas != NULL)
	{
		/*
		printf("Encontro la tabla, el pid es %d, la direccion %p \n", reg_tabla_tablas->pid, reg_tabla_tablas->direc_tabla_proc);
		int c =(reg_tabla_tablas->direc_tabla_proc)->elements_count;
		printf ("cantidad de elementos: %d \n", c);
		*/
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

/*
 * KOLO
 * No pongo semaforos porque unicamente se la llama en leerEnMemReal que ya tiene el sem
 */
void lectura(t_header * proceso_entrante, t_list * tabla_adm, char * memoria_real, char * contenido, t_list * TLB, process_pag * pagina_proceso)
{
	/* BUSCO LA TABLA DEL PROCESO EN LA LISTA DE TABLAS DE PROCESOS POR EL NUMERO DE PID
	numero_de_pid = proceso_entrante->PID;
	t_tabla_adm * registro_tabla_proc = list_find(tabla_adm,(void*) elNodoTienePidIgualA);
	t_list * lista_proceso = registro_tabla_proc->direc_tabla_proc;

	//BUSCO LA ENTRADA DE ESA PAGINA EN LISTA_PROCESO Y LA CARGO
	process_pag * pagina_proceso = list_find(lista_proceso,(void*)_numeroDePagina);
	*/
	// preguntar!! CREO LA ENTRADA DE LA PAGINA A LA TABLA DE PROCESO

	// TRAIGO EL PRIMER MARCO VACIO DE MI MEMORIA PARA ALMACENAR EL CONTENIDO A LEER
	t_marco_hueco * marco_vacio = (t_marco_hueco*)listaFramesHuecosMemR->head;

	// VER QUE ONDA ESTO, NO TIENE LA PAGINA YA?
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
	{
	 	actualizarTlb(proceso_entrante->PID, proceso_entrante->pagina_proceso, pagina_proceso->direccion_fisica, TLB);
	}
	pthread_mutex_unlock (&mutexTLB);
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
		recv(serverSocket, &flag, sizeof(int),0);

		if(flag==1) //si no hubo error
		{
			if(header->type_ejecution==0) //si hice una lectura, devuelve la pag
			{
				recv(serverSocket, (void *)contenido, miContexto.tamanioMarco,0);
			}
		}
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
	t_tabla_adm * registro_tabla_proc = list_find(tabla_adm, (void*)_numeroDePid);

	if (registro_tabla_proc != NULL)
	{
		printf("ENCONTRE UN PROCESO PARA MATAR\n");
		printf ("LA TABLA DE TABLAS DE PROCESOS TIENE %d ELEMENTOS ANTES DE MATAR \n", tabla_adm->elements_count);
		t_list * tabla_proceso = registro_tabla_proc->direc_tabla_proc;

		if (tabla_proceso != NULL)
		{
			printf("ENCONTRE LA TABLA DEL PROCESO A MATAR \n");

			int cantidad_paginas = tabla_proceso->elements_count;
			int x = 0;
			while (x < cantidad_paginas)
			{
				process_pag * pagina_removida = list_remove(tabla_proceso, 0);

				bool _numMarco (void * p)
				{
					return(*(char *)p == pagina_removida->marco);
				}

				if (strcmp(pagina_removida->direccion_fisica,"Swap"))
				{
					printf("ENCONTRO PAGINA A REMOVER QUE TENIA UN MARCO ASIGNADO \n");
					t_marco * marco_a_remover = list_remove_by_condition(listaFramesMemR, (void*)_numMarco);

					//AGREGO EL MARCO AHORA HUECO, A LA LISTA DE MARCOS HUECOS
					// SIGUE TENIENDO SU DIRECCION Y SU NUMERO DE MARCO, NO IMPORTA EN QUE LISTA ESTE
					list_add(listaFramesHuecosMemR, marco_a_remover);
				}
				x++;
			}

			// ELIMINO TODOS LOS ELEMENTOS DE LA TABLA Y LA TABLA
			list_destroy_and_destroy_elements(tabla_proceso, (void *)pag_destroy);
			//ELIMINO LA REFERENCIA DE ESA TABLA DE PROCESO, DESDE LA TABLA DE PROCESOS
			list_remove_by_condition(tabla_adm, (void*)_numeroDePid);
			printf ("LA TABLA DE TABLAS DE PROCESOS TIENE %d ELEMENTOS DESPUES DE MATAR \n", tabla_adm->elements_count);
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
		list_remove_by_condition(TLB,(void*)_numeroDePid);
		printf("ELEMENTOS DE LA TLB DESPUES DE MATAR -> %d \n", TLB->elements_count);
	}
	return 1;
}

int verificarTlb (t_list * TLB, int tamanio_msg, char * message, t_header * pagina)
{
	int * parametro = malloc(sizeof(int));
	t_tlb * registro_tlb = buscarEntradaProcesoEnTlb(TLB, pagina, parametro);

	if (registro_tlb != NULL)
	{
		log_info(logger, "TLB HIT pagina: %d", registro_tlb->pagina);
		strcpy (registro_tlb->direccion_fisica, message);
		//printf( "ESCRIBI : %s EN LA PAGINA %d PID %d Y DIRECCION %p \n", registro_tlb->direccion_fisica, registro_tlb->pagina, registro_tlb->pid, registro_tlb->direccion_fisica);
		return 1;
	}else
	{
		// SI NO LA ENCONTRO RETORNO 0
		log_info(logger, "TLB MISS");
		//printf ("NO ENCONTRE ESA PAGINA EN LA TLB\n");
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
				//printf("ENCONTRE LA PAGINA %d Y SU DIRECCION ES --> %p Y SU POSICION %d \n ", reg_tlb->pagina, reg_tlb->direccion_fisica, x);
				*posicion = x;
				return reg_tlb;
			}
		}
		x++;
	}
	return NULL;
}

// ARREGLAR ESTA FUNCION, ESTA TODO EL CODIGO DUPLICADO ESCRITURA/LECTURA
/*
 * KOLO
 * No pongo semaforo porque la llama una sola vez, y ya tiene el sem ahi
 */
int swapeando(t_list* tablaProceso,t_list* tabla_adm , t_list * TLB, char * mensaje, int serverSocket, t_header * header)
{
	// TRAIGO LA PRIMER PAGINA QUE SE HAYA CARGADO EN MEMORIA, LA PONGO EN NULL Y SE LA ENVIO AL SWAP
	process_pag * pagina_a_remover = traerPaginaARemover(tablaProceso);
	log_info(logger, "Acceso a swap: Voy a swapear para traer la pagina %d porque no tengo lugar para este proceso", header->pagina_proceso);
	sleep(miContexto.retardoMemoria);

	int num_pag_to_remove = pagina_a_remover->pag;

	bool _numeroDePagina (void * p)
	{
		return(*(int*)p == num_pag_to_remove);
	}

	// ACTUALIZO LA PAGINA A SWAPEAR, DIRECCION_FISICA = NULL
	//ESTO ESTA PARA EL ORTO, BUSCAR UNA FORMA MAS EFICIENTE

	process_pag * paginaASwapear = list_remove_by_condition(tablaProceso, (void*)_numeroDePagina);
	list_add(tablaProceso, pag_proc_create(paginaASwapear->pag, "Swap", -1) );


	// SI SE TRATA DE UNA ESCRITURA
	if (header->type_ejecution == 1)
	{
		t_header * header_escritura = crearHeaderEscritura( header->PID, paginaASwapear->pag, miContexto.tamanioMarco);

		int status_escritura = envioAlSwap(header_escritura, serverSocket, paginaASwapear->direccion_fisica);

		// LE PIDO LA PAGINA QUE QUIERO ESCRIBIR Y LA AGREGO AL FINAL DE LA LISTA
		t_header * header_lectura = crearHeaderLectura(header);

		char * contenido = malloc(miContexto.tamanioMarco);
		int status_lectura = envioAlSwap(header_lectura, serverSocket, contenido);

		int num_pag = header->pagina_proceso;

		bool _numeroDePag (void * p)
		{
			return(*(int*)p == num_pag);
		}

		// La agrego al final con la direccion del marco de la pagina que swapee ( ALGORITMO FIFO )
		if ( !strcmp(miContexto.algoritmoReemplazo, "FIFO"))
		{
		list_remove_by_condition(tablaProceso, (void*)_numeroDePag);
		list_add(tablaProceso, pag_proc_create(header->pagina_proceso, paginaASwapear->direccion_fisica, paginaASwapear->marco));
		} // SINO VEO LOS OTRO ALGORITMOS  ( DESPUES VEO )


		// Escribo en mi pagina swapeada el contenido a escribir
		log_info(logger, "Escribo el marco de mi pagina swapeada para escribir");
		sleep(miContexto.retardoMemoria);
		strcpy(paginaASwapear->direccion_fisica, mensaje );

		if ( status_lectura == 1)
			return 1;
		else
			return 0;
		// SI SE TRATA DE UNA LECTURA
	}else if(header->type_ejecution ==0)
	{
		int status_lectura = envioAlSwap(header, serverSocket, NULL);
		log_info(logger, "Escribo el marco de mi pagina swapeada para leer");
		sleep(miContexto.retardoMemoria);
		strcpy(paginaASwapear->direccion_fisica, mensaje );

		int num_pag = header->pagina_proceso;

		bool _numeroDePag (void * p)
		{
			return(*(int*)p == num_pag);
		}

		// La agrego al final con la direccion del marco de la pagina que swapee
		process_pag * paginaALeer = list_remove_by_condition(tablaProceso, (void*)_numeroDePag);
		list_add(tablaProceso, pag_proc_create(header->pagina_proceso, paginaASwapear->direccion_fisica, paginaASwapear->marco));


		if ( status_lectura == 1)
			return 1;
		else
			return 0;

	}
	return 1;
}

void actualizarTablaProceso(t_list * tabla_proceso, int num_pagina, char * direccion_marco, int num_marco)
{
	bool _numeroDePagina (void * p)
	{
		return(*(int*)p == num_pagina);
	}

	list_remove_by_condition(tabla_proceso, (void*)_numeroDePagina); // ¿ ME DEVUELVE LO QUE REMUEVE ESTA FUNCION?
	process_pag * pagina = pag_proc_create(num_pagina, direccion_marco, num_marco);
	list_add(tabla_proceso,pagina);
}

process_pag * traerPaginaARemover(t_list * tablaProceso)
{
	// SI ALGORITMO == FIFO
	if (!strcmp(miContexto.algoritmoReemplazo, "FIFO"))
	{
		return primerPaginaCargada(tablaProceso);
	// SI ALGIRTMO == CLOCK
	}else if (!strcmp(miContexto.algoritmoReemplazo, "CLOCK"))
	{
		// return
	// SI ALGORITMO == LRU
	}else
	{

	}
	return NULL;
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

	if(entrada_tlb!=NULL)
	{
		// Verificando que ande la condicion
		printf("Ya estaba cargada en TLB ");
		//t_tlb * removido = list_remove(TLB, posicion);
		//list_add(TLB, removido);

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
			list_remove_and_destroy_element(TLB, 0, (void*)reg_tlb_destroy);
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

	if (!strcmp(miContexto.tlbHabilitada,"SI"))
	{
		puts("Uy, voy a vaciar la TLB\n");
		printf("La TLB tiene %d elementos \n", TLB->elements_count);
		int i=0;
		pthread_mutex_lock (&mutexTLB);
		for(;i<TLB->elements_count;i++)
		{
			list_remove_and_destroy_element(TLB, i, (void *)reg_tlb_destroy);
		}
		pthread_mutex_unlock (&mutexTLB);
	}
	else
	{
		puts("La TLB NO esta habilitada campeon.\n");
	}

	printf("La TLB tiene %d elementos \n", TLB->elements_count);
	puts("Mostrame esto gato \n");
}

/*
 * Cuando se recibe esta señal, se debe limpiar completamente la memoria principal,
 * actualizando los bits que sean necesarios en las tablas de páginas de los diferentes procesos
 */
void limpiarMemoria(void * args)
{
	parametros * param;
	param = (parametros * ) args;

	puts("Recibi SIGUSR2 \n");
	//Vacio la memoria
	pthread_mutex_lock (&mutexMem);
	strcpy(param->memoria,"\0");

	//Actualizo las listas
	int i = 0, j = 0;
	for(;i<param->tabla_adm->elements_count;i++) //Recorro la tabla de tablas
	{
		//Traigo una tabla
		t_tabla_adm * entrada_tabla_tablas = list_get(param->tabla_adm,i);
		t_list * tablaProceso = entrada_tabla_tablas->direc_tabla_proc;

		for(;j<tablaProceso->elements_count;j++) //Recorro la tabla de procesos
		{
			process_pag * pagina_proc = list_get(tablaProceso, j); //Traigo una pagina
			//Actualizo la pagina
			pagina_proc->direccion_fisica = NULL;
			pagina_proc->marco=-1;
		}
	}
	//Actualizo marcos
	list_destroy_and_destroy_elements(listaFramesMemR,(void *)marco_destroy);
	list_destroy_and_destroy_elements(listaFramesHuecosMemR,(void *)marco_hueco_destroy);
	listaFramesMemR = crearListaFrames();
	listaFramesHuecosMemR = crearListaHuecosFrames(miContexto.cantidadMarcos, miContexto.tamanioMarco, param->memoria);
	pthread_mutex_unlock (&mutexMem);
	puts("Mostrame esto");
}

/*
 * Cuando recibe esta señal se deberá realizar un volcado (dump) del contenido de la memoria principal,
 * en el archivo log de Administrador de Memoria, creando para tal fin un proceso nuevo.
 * Se recomienda usar fork().
 */
void dumpEnLog(char * memoria_real, t_list * tablaAdm)
{
	puts("Recibi SIGPOLL");
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
	puts("Termine dump");

}
//-----------------------------------------------------------//

// --------------ENTRADAS A LA TABLA DE PROCESO ------------ //
process_pag * pag_proc_create (int pagina, char * direccion_fisica, int marco)
{
 process_pag * reg_pagina = malloc(sizeof(process_pag));
 reg_pagina->pag = pagina;
 reg_pagina->direccion_fisica = direccion_fisica;
 reg_pagina->marco = marco;
 return reg_pagina;
}

void pag_proc_destroy(process_pag * self)
{
 free(self);
}
// --------------------------------------------------------//


