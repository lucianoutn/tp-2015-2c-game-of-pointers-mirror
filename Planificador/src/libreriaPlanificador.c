/*
 * libreriaPlanificador.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#include "libreriaPlanificador.h"



//A continuancion las funciones basicas para crear una nueva cola FIFO
//Funcion que permite añadir elementos a la cola
void encolar (t_pcb *cabecera, t_pcb *valor)
{
	t_pcb *nuevo = malloc(sizeof(t_pcb));
	nuevo=valor;

	if (cabecera==NULL)
	{
		nuevo->sig=nuevo;
	}
	else
	{
		nuevo->sig= cabecera->sig;
		cabecera->sig= nuevo;
	}

	cabecera=nuevo;
};

//Funcion que permite quitar elementos de la cola, devuelve el nodo que saca.
t_pcb desencolar (t_pcb *cabecera)
{
	t_pcb *ret = malloc(sizeof(t_pcb));
	ret= cabecera;
	if(cabecera==cabecera->sig)
	{
		free (cabecera);
	}

	else
	{
		t_pcb *aux = malloc(sizeof(t_pcb));
		aux= cabecera->sig;
		cabecera->sig=aux->sig;
		free(aux);
	}

	return *ret;
};


//Funcion que muestra la consola por pantalla con las opciones a enviar a la CPU
void consola ()
{
	//variables auxiliares para el uso de la consola
	int socket_instrucciones, caracter, enviar;
	char message[PACKAGESIZE]; //tamaño maximo de los paquetes

	//controla que el usuario no quiera salir
	while(strcmp(message,"salir\n") !=0)
	{
		//Muestra las conexiones con las CPUS disponibles
		puts("Elija CPU: ¡¡¡¡SOLO NUMEROS!!!\n");
		int j = 0, i;
		while ( j < 5) {
			printf("CPU n°:%d, puerto: %d\n",j+1,conexiones.CPU[j]);
			j++;
		}

		//Permite elegir la conexion con el CPU deseado
		scanf("%d", &caracter);
		enviar =1;
		switch (caracter) {
			case 1:
				socket_instrucciones = conexiones.CPU[0];
				break;
			case 2:
				socket_instrucciones = conexiones.CPU[1];
				break;
			case 3: socket_instrucciones = conexiones.CPU[2];
				break;
			case 4:
				socket_instrucciones = conexiones.CPU[3];
				break;
			case 5:
				socket_instrucciones = conexiones.CPU[4];
				break;
			default:
				{puts("CPU NO VALIDA!"); enviar=0; caracter=0;};
				break;
		}

		//Permite el envio de paquetes, dependiendo si la opcion elegida es valida
		printf("Ya puede enviar instrucciones.\nEscriba 'correr programa' para enviar una señal al CPU\n'cpu' para cambiar de CPU\n'salir' para cerrar los procesos\n");
		while(enviar){
			fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
			if (!strcmp(message,"cpu\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
			if (!strcmp(message,"correr programa\n")) send(socket_instrucciones, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
			if (!strcmp(message,"salir\n")){ for(i=0;i<5;i++) {send(conexiones.CPU[i], message, strlen(message) + 1, 0);}; break;};
		}

	}
}
