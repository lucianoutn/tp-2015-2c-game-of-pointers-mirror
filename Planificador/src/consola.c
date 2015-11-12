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
	"finalizar",
	"ps",
	"cpu",
	"salir"
};

//Funcion que muestra la consola por pantalla con las opciones a enviar a la CPU
void consola (void* arg)
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


	do{				//valido comando ingresado
		fflush(stdin);		//limpia el buffer del teclado
		printf("\nIngrese el comando deseado o ayuda para conocer los comandos posibles\n");
		command = leeComando();
		}// leeComando lee la palabra y me devuelve un comando del enum
	while (command ==-1);

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
				sem_post(&semConsola); //habilito de nuevo la consolita
				break;
			}
			case correr:
			{
				//el hilo debe enviar una señal al planificador para que solicite la ruta al usuario
				//sem_post(semProduccionMsjs); //este sem hay q colocarlo adentro de inciarPlanificador();
				orden=0;
				sem_post(&ordenIngresada); //habilita el swich case en planificador.c
				break;
			}
			case finalizar:
			{
				int pidAfinalizar;
				t_pcb *pcb;
				printf("\nFinalizar tiene efecto solamente cuando el algoritmo de planificacion es RR\n");// por fifo tmb finaliza pero no se ve un "efecto"
				printf("Ingrese numero de PID para FINALIZAR\n");
				fflush(stdin);	//limpia el buffer x el enter colgado
				char buffer[1024];
				fgets(buffer,sizeof(buffer),stdin);
				sscanf(buffer, "%d", &pidAfinalizar);
				//printf("Se finalizara el PID: %d\n", pidAfinalizar);
				//marco el pcb para que finalize en la siguiente rafaga de ejecución


				int buscoPid(t_pcb *pcb) {		 //creo la funcion para usar ORDEN SUPERIOR
					return pcb->PID == pidAfinalizar;
				}

				pcb = list_find(lstPcbs, (void*) buscoPid); //busco el pcb
				//lo marco para finalizar
				//aca quiero lograr que en el pcb->campo finalizar ponga un 1
				//printf("estoy leyendo el pcb: %d\n valor de finalizar: %d\n", pcb->PID, pcb->finalizar); //test
				pcb->finalizar = true;
				printf("PID: %d FINALIZADO (en proximo ciclo de cpu)\n", pidAfinalizar);
				//printf("%d", pcb->finalizar);// test

				sem_post(&semConsola); //vuelvo a habilitar la consolita
				break;
			}
			case ps:
			{
				//printf("Este comando falta desarrollar\n");
				//system("ps");
				puts("\nEstado de Procesos\n");

				void imprimePS(t_pcb *pcb) {		 //creo la funcion para usar ORDEN SUPERIOR
					printf("mProc %d: <%s> -> %s\n", pcb->PID, ruta(pcb->PID), estadoActual(pcb->estado));
				}
				//mapeo la funsion imprimePS en cada nodo de la lista
				list_iterate(lstPcbs,(void*)imprimePS);

				sem_post(&semConsola); 	//vuelvo a habilitar la consolita
				break;
			}
			case cpu:
			{
				int i;
				printf("\nUSO DE LA CPU\n");
				for(i=1; i<=miContexto.cantHilosCpus; i++){
					printf("CPU %d: %d%\n",i,100);
				}
				sem_post(&semConsola);  //vuelvo a habilitar la consolita
				break;
			}
			case salir:
			{
				orden=1;

				for(m=0;m<miContexto.cantHilosCpus;m++)
				{
					t_headcpu header;
					header.tipo_ejecucion=0;
					send(conexiones.CPUS[m].socket, &header,sizeof(t_headcpu),0);
					sem_post(semProduccionMsjs);	//habilita el proceso CPU

				}

				sem_post(&semSalir);
				sem_post(&ordenIngresada);	//habilita el swich case en planificador.c
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
			sem_wait(&semConsola);	//espera la correcta ejecucion de un comando anterior para ejecutar uno nuevo
			usleep(500);		//esta puesto a modo estetico, xq a veces se imprime otro mensaje de otro hilo antes.

			do{				//valido comando ingresado
					fflush(stdin);	//limpia el buffer del teclado
					printf("\nIngrese el comando deseado o ayuda para conocer los comandos posibles\n");
					command = leeComando();
					}// leeComando lee la palabra y me devuelve un comando del enum
			while (command ==-1);

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
