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
	"iniciar",
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
	iniciar,
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

void procesarPCB(t_queue * cola_ready, char * path, int socketCPU)
{
	CPUenUso=true;
	/*enviar(PCB) a la CPU
	 *esperar a que la CPU termine de procesar
	 *recibe respuesta de la CPU loguea y termina
	 */

	t_pcb * pcb;
	//armo pcb
	pcb->PID = max_PID +1;
	pcb->instructionPointer = 0;
	pcb->numInstrucciones = 0;
	pcb->prioridad=0;
	pcb->permisos=0;
	pcb->ruta = path;
	max_PID = pcb->PID;

	CPUenUso=false;

	//encolo a la cola de ready
	queue_push(cola_ready, pcb);
	//le envio a la CPU el t_headcpu y el PCB

	t_msj msj;
	msj.headMSJ.tipo_ejecucion = 1;
	msj.headMSJ.tamanio_msj = sizeof(t_pcb);
	msj.pcbMSJ = pcb;
	send(socketCPU, &msj,sizeof(t_msj),0);
	puts("Mensaje enviado\n");

}


//Funcion que muestra la consola por pantalla con las opciones a enviar a la CPU
int consola (t_queue * cola_ready)
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
				iniciarPlanificador(cola_ready);
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


void iniciarPlanificador(t_queue* cola_ready)
{
	int primeraVez = 1;
	int enviar =1 ;
	int cpuElegida;
	int socketCPU = conexiones.CPU[0];
	char * message=malloc(150);
	//t_pcb *PCB=(t_pcb*)malloc(sizeof(t_pcb));
	//PCB->ruta=path;
	// encolar(inicio, PCB);
	//controla que el usuario no quiera salir
	while(strcmp(message,"menu\n") !=0)
	{
		if(!primeraVez)
		{
			//Muestra las conexiones con las CPUS disponibles
			puts("Elija CPU: ¡¡¡¡SOLO NUMEROS!!!\n");
			int j = 0;
			while ( j < 1)
			{
				printf("CPU n°:%d, puerto: %d\n",j+1,conexiones.CPU[j]);
				j++;
			}
			//Permite elegir la conexion con el CPU deseado
			scanf("%d", &cpuElegida);
			socketCPU = conexiones.CPU[cpuElegida];
		}

		//Permite el envio de paquetes, dependiendo si la opcion elegida es valida
		printf("Escriba 'correr PATH' para correr un proceso en el CPU (por defecto la primera)\n'cpu' para cambiar de CPU\n'menu' para volver al menu principal\n");
		while(enviar)
		{
			fflush(stdin);
			fgets(message, 150, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
			if (!strcmp(message,"cpu\n"))
			{
				primeraVez = 0;
				enviar = 0;			// Chequeo que el usuario no quiera salir
				printf("Comando no implementado");
			}
			//MODIFICAR QUE PASA SI LA CPU ESTA EN USO
			/*
			if(CPUenUso)
			{
				//creo el hilo con la funcion procesarPCB (PCB);
				if(pthread_create(&hCPU1,NULL,procesarPCB,&PCB)<0)
					perror("Error HILO CPU!");
			}
			*/

			/*
			 * Protocolo de mensajes Planificador -CPU
			 * Para poder entender los distintos tipos de mensajes que se envia, mandamos primero
			 * un header t_headcpu.
			 * tipo_ejecucion: 0 - salir
			 * 				   1 - correr programa
			 * 				   	   envia PCB
			 * tamanio_mensaje: tamanio del char * o del PCB
			 */
			if (!strcmp(string_substring(message,0,7),"correr "))
			{
				char * path= malloc(143);
				//Tomo el path
				path = string_substring_from(message, 7); //inclye el \n
				//PROCESO EL PCB
				procesarPCB(cola_ready, path,socketCPU);

				free(path);
			}
			if (!strcmp(message,"menu\n"))
			{
				//Salgo
				break;
			}
		}
	}
}
