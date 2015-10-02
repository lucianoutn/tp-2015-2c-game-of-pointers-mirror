/*
 * libreriaPlanificador.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#include "libreriaPlanificador.h"

const char* allCommands[] =
	{
		// esto define las palabras de los comandos aceptados
		// lo que el usuario ingrese va a ser comparado con esto
		// TODOS LOS COMANDOS deben estar en minúscula para que lo reconozca bien sin importar como lo ingrese el usuario
		"ayuda",
		"correr path",
		"finalizar PID",
		"ps",
		"cpu",
		"salir"
	};
	enum Commands
	{
		// esto define los comandos en la forma en que los vamos a manipular internamente
		// son enteros que representan a un estado, que no sean solo números ayuda a la lectura
		// pero es solo de uso interno
		// si se ingresa un nuevo comando o se modifica, se debe modificar también su representación
		// del lado del usuario, en la lista de arriba.
		ayuda = 0,
		correr,
		finalizar,
		ps,
		cpu,
		salir,
		enter
	};


/****************************************************************
				Lee el comando y lo busca entre los reconocidos
****************************************************************/
		int leeComando(void)
		{
			int c, i = 0;
			char palabra[WORD_SIZE] = { NULL };

			while ((c = getchar()) != '\n') //leo hasta identificar enter
			{
				if (i > WORD_SIZE)
				{
					// si me pasé del largo de comando permitido devuelvo -1 para que el autómata
					// identifique que no se ha ingresado un comando permitdo
					printf("No se pudo encontrar el comando especificado\n");
					printf("Ingrese el comando deseado o help para conocer los comandos posibles\n");
					return -1;
				}
				else
				{
					// si sigo dentro de rango guardo el caracter en la palabra a comparar con comandos
					palabra[i] = tolower(c); //guardo el caracter ingresado para concatenar la palabra
					// si es mayuscula lo paso a minuscula para no tener que comparar todos los casos
				}
				i++;
			}

			if (!i)
			{
				// si estoy acá el tipo apretó enter sin entrar nada, reflejo el enter y no hago nada para
				// mantener el parecido del comportamiento de una consola
				return enter;
			}

			//si estoy aca es porque detecte un enter! Entonces mi palabra deberia estar lista para comparar
			i = 0; // reutilizo i porque el primer while ya no la necesita

			// comparo la palabra con cada uno de los comandos permitidos, hasta salir que es el ultimo
			while (i <= salir && strcmp(allCommands[i], palabra))
			{
				// si no hay match, comparo con el próximo
				i++;
			}
			// tengo dos posibles causas de haber salido del while, diferencio ambas
			if (i > salir)
			{
				// si esoy aca es porque lo ingresado no coincide con alguno de los comandos -> devuelvo error
				// no informo que no entendí el comando porque de eso se encarga el autómata cuando sale por default
				return -1;
			}
			else
			{
				// si estoy aca es porque ¡hay match!
				// devuelvo el id del comando encontrado!
				return i;
			}
		}



void traigoContexto()
{
 // LEVANTO EL ARCHIVO CONFIG Y VERIFICO QUE LO HAYA HECHO CORRECTAMENTE /
 t_config * config_planificador = config_create("resources/config.cfg");

 if( config_planificador == NULL )
 {
  puts("No se cargo el archivo de configuracion");
  abort();
 }

 // OBTENGO CONFIGURACION DEL CONFIG /
 miContexto.puertoEscucha = config_get_string_value(config_planificador, "PUERTO_ESCUCHA" );
 miContexto.algoritmoPlanificacion = config_get_string_value(config_planificador, "ALGORITMO_PLANIFICACION" );
}



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

void *procesarPCB(t_pcb *inicio)
{
	CPUenUso=true;
	//enviar(PCB) a la CPU
	//esperar a que la CPU termine de procesar
	//recibe respuesta de la CPU loguea y termina
	t_pcb PCB= desencolar(inicio);
	CPUenUso=false;
}


//Funcion que muestra la consola por pantalla con las opciones a enviar a la CPU
void consola (t_pcb *inicio)
{
	//variables auxiliares para el uso de la consola
	int socket_instrucciones, caracter, enviar;
	char message[PACKAGESIZE]; //tamaño maximo de los paquetes
	char *buffer;
	int command;
	int i;
	//muestra cartel de consola
	system("clear");
	printf("_____________________________________________________________________________________________\n");
	printf("                     Consola VERSION 2.0 | Grupo: Game of Pointers\n");
	printf("_____________________________________UTN_|_Sistemas Operativos_______________________________\n");

	printf("\nIngrese el comando deseado o ayuda para conocer los comandos posibles\n");

	command = leeComando(); // read lee la palabra y me devuelve un comando del enum
	while(command != salir) // Itero por siempre
	{
		switch (command)
		{
			case ayuda:
			{
				printf("\n\nAyuda: Muestra todos los comandos disponibles\n");
				for (i = 0; i <= salir; i++)
				{
					printf("%02d:\t%s\n", i + 1, allCommands[i]);
				}
				break;
			}
			case correr:
			{
				 t_pcb *PCB=(t_pcb*)malloc(sizeof(t_pcb));
				 //PCB->ruta=path;
				 encolar(inicio, PCB);
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
						case 3:
							socket_instrucciones = conexiones.CPU[2];
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
						//en esta seccion se crea el PCB y se crea el hilo para enviar datos a la consola
						if(!CPUenUso)
						{
						//creo el hilo con la funcion procesarPCB (PCB);
						}

						if (!strcmp(message,"correr programa\n")) send(socket_instrucciones, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
						if (!strcmp(message,"salir\n")){ for(i=0;i<5;i++) {send(conexiones.CPU[i], message, strlen(message) + 1, 0);}; break;};
					}
				}
				break;
			}

			case finalizar:
			{
				printf("Este comando todavia no ha sido implemenado\n");
				break;
			}
			case ps:
			{
				printf("Este comando todavia no ha sido implemenado\n");
				system("ps");
				break;
			}
			case cpu:
			{
				printf("Este comando todavia no ha sido implemenado\n");
				break;
			}
			case enter:
			{
				// no haga nada, que se refleje el enter en la consola
				break;
			}
			default:
			{
				// No se que tan necesario es esto porque nosotros devolvemos el comando ingresado, pero bueno...
				printf("No se pudo encontrar el comando especificado\n");
				printf("Ingrese el comando deseado o help para conocer los comandos posibles\n");
				break;
			}
			// esto es porque al final de la ejecución de un comando, puede quedar en el flujo estandar el ingreso
			// de algo (generalmente un enter que es lo ultimo que leemos) por teclado, por lo que el autómata interpretaría
			// que se terminó de ingresar un comando y trataría de entenderlo. Para evitar eso al final de cada lectura
			// vaciamos el contenido en el flujo stdin
		}
			fflush(stdin);
			printf("\nIngrese el comando deseado o ayuda para conocer los comandos posibles\n");
			command = leeComando(); // read lee la palabra y me devuelve un comando del enum
	}
}
