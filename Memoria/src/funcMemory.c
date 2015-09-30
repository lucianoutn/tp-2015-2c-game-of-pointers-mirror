/*
 * funcMemory.c
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */


#include "funcMemory.h"

void traigoContexto()
{
 // LEVANTO EL ARCHIVO CONFIG Y VERIFICO QUE LO HAYA HECHO CORRECTAMENTE /
 t_config * config_memory = config_create("resources/config.cfg");

 if( config_memory == NULL )
 {
  puts("Final felize");
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

void ejecutoInstruccionEnCache(t_header registro_prueba, char * memoria_cache, t_list * TLB)
{
 switch (registro_prueba.type_ejecution)
   {
   case 0:
   printf ("Se recibio orden de lectura\n");
   meConectoAlSwap(registro_prueba,NULL);
    break;
   case 1:
   printf ("Se recibio orden de escritura\n");
    break;
   case 2:
    printf("Se recibio orden de inicializacion \n");
    iniciarEnCache(registro_prueba, TLB,  memoria_cache);
    break;
   case 3:
   printf ("Se recibio orden de finalizacion de proceso :) \n");
   iniciarEnCache(registro_prueba, TLB, memoria_cache);
   int cant_elem = TLB->elements_count;
   printf ("LA TLB TIENE %d ELEMENTOS DESPUES DE INICIAR \n", cant_elem );
   numero_de_pid = registro_prueba.PID;
   //TLB->head
   list_remove_by_condition(TLB, elNodoTienePidIgualA );
   cant_elem = TLB->elements_count;
   printf ("LA TLB TIENE %d ELEMENTOS DESPUES DE FINALIZAR \n", cant_elem );
    break;
   default:
   printf ("El tipo de ejecucion recibido no es valido\n");
    break;
   }
}

void ejecutoInstruccionEnMem(t_header registro_prueba, char * memoria_real, t_list * tabla_mem_real)
{
 switch (registro_prueba.type_ejecution)
   {
   case 0:
   printf ("Se recibio orden de lectura\n");
    break;
   case 1:
   printf ("Se recibio orden de escritura\n");
    break;
   case 2:
    printf("Se recibio orden de inicializacion \n");
    iniciarEnMemReal(registro_prueba, tabla_mem_real,  memoria_real);
    break;
   case 3:
   printf ("Se recibio orden de finalizacion de proceso :) \n");
    break;
   default:
   printf ("El tipo de ejecucion recibido no es valido\n");
    break;
   }
}

void iniciarEnCache(t_header registro_prueba, char * TLB, char * memoria_cache)
{
 int cant_paginas = registro_prueba.pagina_proceso;
 int x = 0, bytes = 0, a = 0;
 printf ("CANTIDAD DE PAGINAS: %d \n", cant_paginas);
 // MIENTRAS HAYA PAGINAS DEL PROCESO PARA INICIAR /
 while (x != cant_paginas)
 {
  printf("Entre %d veces + 1 \n", x);
  ttlb * reg_input = malloc(sizeof(ttlb));
  reg_input->PID = registro_prueba.PID;
  reg_input->marco = x+1;
  reg_input->direc_mem = memoria_cache + bytes;
  bytes = bytes + miContexto.tamanioMarco;
  printf ("LA DIRECCION DE MEMORIA DEL PROCESO ES: %p \n", reg_input->direc_mem);
  a=list_add(TLB,reg_input);
  x++;
 }
}

void iniciarEnMemReal(t_header registro_prueba, char * tabla_mem_real, char * memoria_real)
{
 int cant_paginas = registro_prueba.pagina_proceso;
 int x = 0, bytes = 0, a = 0;
 printf ("CANTIDAD DE PAGINAS: %d \n", cant_paginas);
 // MIENTRAS HAYA PAGINAS DEL PROCESO PARA INICIAR /
 while (x != cant_paginas)
 {
  printf("Entre %d veces + 1 \n", x);
  //t_tabla_mr * reg_input = malloc(sizeof(ttlb));
  //reg_input->PID = registro_prueba.PID;
  //reg_input->marco = x+1;
  //reg_input->direc_mem = memoria_real + bytes;
  bytes = bytes + miContexto.tamanioMarco;
  //printf ("LA DIRECCION DE MEMORIA DEL PROCESO ES: %p \n", reg_input->direc_mem);
  //a=list_add(TLB,reg_input);
  x++;
 }
}

void finalizarProceso(t_header reg)
{
}


bool elNodoTienePidIgualA(int * pid_number)
{
 return (*pid_number == numero_de_pid);

}

void meConectoAlSwap(t_header registro_prueba, char * mensaje)
{
 int serverSocket;

 conexionAlServer(&serverSocket, miContexto.puertoCliente);

 //int enviar = 1;
 //char message[PACKAGESIZE];

 printf("Conectado al servidor Swap\n. Bienvenido al sistema, ya puede enviar mensajes\n. Escriba 'exit' para salir\n");

 //while(enviar)
 //{
  //fgets(message, PACKAGESIZE, stdin);   // Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
  //if (!strcmp(message,"exit\n")) enviar = 0;   // Chequeo que el usuario no quiera salir
  //if (enviar) send(serverSocket, package, strlen(package) + 1, 0);  // Solo envio si el usuario no quiere salir.
  send(serverSocket, &registro_prueba, sizeof(t_header), 0);
  send(serverSocket, mensaje, strlen(mensaje), 0);
  //enviar = 0;
 //}

 close(serverSocket);
}

