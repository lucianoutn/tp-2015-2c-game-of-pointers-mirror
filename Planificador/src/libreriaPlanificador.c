/*
 * libreriaPlanificador.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#include "libreriaPlanificador.h"

//Funcion encargada de acceptar nuevas peticiones de conexion
void escuchar (){
	int i =0;

	while( i<=5 ) //limite temporal de 5 CPUS conectadas
	{
		//guarda las nuevas conexiones para acceder a ellas desde cualquier parte del codigo
		conexiones.CPU[i] = accept(conexiones.socket_escucha, (struct sockaddr *) &conexiones.direccion, &conexiones.tamanio_direccion);
		if(conexiones.CPU[i]==-1)
		{
			perror("ACCEPT"); //control error
		}
		puts("NUEVO HILO ESCUCHA!\n");
		i++;
	}

}

//Funcion que permite configurar la conexion y crear el socket de escucha nuevo, lo agrega al a variable conexiones
void crearSocket ()
{
	struct addrinfo hints; //estructura que almacena los datos de conexion del Planificador
	struct addrinfo *serverInfo; //estructura que almacena los datos de conexion de la CPU

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion

	//se crea un nuevo socket que se utilizara para la conexion con el CPU
	conexiones.socket_escucha = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);
	//se comprueba que el socket se creo correctamente
	if (conexiones.socket_escucha == -1)
		perror("SOCKET");

	//se comprueba que la asociacion fue exitosa
	int B = bind(conexiones.socket_escucha, serverInfo->ai_addr, serverInfo->ai_addrlen);
	if (B == -1)
		perror("BIND");

	//funcion que permite al programa ponerse a la espera de nuevas conexiones
	int L = listen(conexiones.socket_escucha, BACKLOG);
	if (L == -1)
		perror("LISTEN");


	//Se calcula el tamaño de la direccion del cliente
	conexiones.tamanio_direccion = sizeof(conexiones.direccion);

	freeaddrinfo(serverInfo); //se libera porque ya no se usa mas

}

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
		int j = 1;
		while ( j < 6) {
			printf("CPU n°:%d, puerto: %d\n",j,conexiones.CPU[j-1]);
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
			if (!strcmp(message,"salir\n")){ send(socket_instrucciones, message, strlen(message) + 1, 0); break;};
		}

	}
}
