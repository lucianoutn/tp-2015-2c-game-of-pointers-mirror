/*
 * consola.c
 *
 *  Created on: 5/10/2015
 *      Author: utnso
 */

#include "libreriaPlanificador.h"
#include "consola.h"

//Funcion que muestra la consola por pantalla con las opciones a enviar a la CPU
int consola ()
{
	//variables auxiliares para el uso de la consola
	char *buffer;
	int command;
	int i,m;
	t_msj msj;
	CPUenUso=false;
	pthread_t hCPU1;
	//muestra cartel de consola
	system("clear");
	printf("_____________________________________________________________________________________________\n");
	printf("                     Consola VERSION 2.0 | Grupo: Game of Pointers\n");
	printf("_____________________________________UTN_|_Sistemas Operativos_______________________________\n");

	printf("\nIngrese el comando deseado o ayuda para conocer los comandos posibles\n");

	command = leeComando(); // read lee la palabra y me devuelve un comando del enum
	while(command != -2) // Itero por siempre
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
			case iniciar:
			{
				//iniciarPlanificador(cola_ready);
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
			case salir:
			{
				for(m=0;m<5;m++)
				{
					msj.headMSJ.tipo_ejecucion = 0;
					msj.headMSJ.tamanio_msj = 0;
					send(conexiones.CPU[m], &msj,sizeof(t_msj),0);
				}
				//command=-2;
				sem_post(&semSalir);
				return -1;
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
	return 0;
}

//Funcion que lee el comando y lo busca entre los reconocidos
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
