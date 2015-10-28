/*
 * consola.c
 *
 *  Created on: 5/10/2015
 *      Author: utnso
 */

#include "libreriaPlanificador.h"
#include "consola.h"

const char* allCommands[] =
{
	// esto define las palabras de los comandos aceptados
	// lo que el usuario ingrese va a ser comparado con esto
	// TODOS LOS COMANDOS deben estar en minúscula para que lo reconozca bien sin importar como lo ingrese el usuario
	"ayuda",
	"correr", // tendria q ser correr PATH directamente acá
	"finalizar PID",
	"ps",
	"cpu",
	"salir"
};

//Funcion que muestra la consola por pantalla con las opciones a enviar a la CPU
void *consola (void* arg)
{
	//variables auxiliares para el uso de la consola
	//char *buffer;
	int command;
	int i,m;

	CPUenUso=false;
	//pthread_t hCPU1;

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
				printf("\n\nAyuda: Comandos disponibles: \n");
				for (i = 0; i <= salir; i++)
				{
					printf("%02d:\t%s\n", i + 1, allCommands[i]);
				}
				sem_post(&semConsola);
				break;
			}
			case correr:
			{
				//el hilo debe enviar una señal al planificador para que solicite la ruta al usuario
				//sem_post(semProduccionMsjs); //este sem hay q colocarlo adentro de inciarPlanificador();
				orden=0;
				sem_post(&ordenIngresada);
				break;
			}

			case finalizar:
			{
				printf("Este comando todavia no ha sido implemenado\n");
				break;
			}
			case ps:
			{
				printf("Este comando falta desarrollar\n");
				system("ps");

				//list_iterate(lstPcbs, void(*closure)(void*));
				//list_add(lstPcbs, NULL);
				sem_post(&semConsola);
				break;
			}
			case cpu:
			{
				printf("Este comando todavia no ha sido implemenado\n");
				sem_post(&semConsola);
				break;
			}
			case salir:
			{
				orden=1;

				for(m=0;m<MAX_CPUS;m++)
				{
					t_headcpu header;
					header.tipo_ejecucion=0;
					send(conexiones.CPUS[m].socket, &header,sizeof(t_headcpu),0);
					sem_post(semProduccionMsjs);

				}

				sem_post(&semSalir);
				sem_post(&ordenIngresada);
				//return (int*)-1; no tiene q retornar nada
				//void pthread_exit(void *retval); //mata el hilo
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
				printf("Ingrese el comando deseado o ayuda para conocer los comandos posibles\n");
				sem_post(&semConsola); //para cuando no reconoce el comando
				break;
			}
			// esto es porque al final de la ejecución de un comando, puede quedar en el flujo estandar el ingreso
			// de algo (generalmente un enter que es lo ultimo que leemos) por teclado, por lo que el autómata interpretaría
			// que se terminó de ingresar un comando y trataría de entenderlo. Para evitar eso al final de cada lectura
			// vaciamos el contenido en el flujo stdin
		}
			sem_wait(&semConsola);
			fflush(stdin);
			printf("\nIngrese el comando deseado o ayuda para conocer los comandos posibles\n");
			command = leeComando(); // read lee la palabra y me devuelve un comando del enum
	}

}

//Funcion que lee el comando y lo busca entre los reconocidos
int leeComando(void)
{
	int c, i = 0;
	char *palabra=(char*)malloc(sizeof(char)*WORD_SIZE);
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
			palabra[i] = (char)tolower(c); //guardo el caracter ingresado para concatenar la palabra
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
