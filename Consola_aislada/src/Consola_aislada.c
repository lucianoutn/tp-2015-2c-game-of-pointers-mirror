/*
 ============================================================================
 Name        : Consola_aislada.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

//#include<ncurses.h>
#include<stdio.h>
#include<stdlib.h>

#define WORD_SIZE 35
#define LARGO_NOMBRE_PATH 60
#define RUTA_LOCAL "/home/utnso/"

int read(void);
/*int crearDir(char v[LARGO_NOMBRE_PATH]);*/

const char* allCommands[] =
{
	// esto define las palabras de los comandos aceptados
	// lo que el usuario ingrese va a ser comparado con esto
	// TODOS LOS COMANDOS deben estar en minúscula para que lo reconozca bien sin importar como lo ingrese el usuario
	"ayuda",
	"correr PATH",
	"finalizar PID",
	"ps",
	"cpu",
	/*"formatearmdfs",
	"eliminararchivo",
	"renombrararchivo",
	"moverarchivo",
	"creardirectorio",
	"eliminardirectorio",
	"renombrardirectorio",
	"copiararchivoenmdfs",
	"copiararchivodesdemdfs",
	"solicitarmd5",
	"verbloque",
	"copiarbloque",
	"borrarbloque",
	"agregarnodo",
	"eliminarnodo",*/
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
	/*formatearMDFS,
	eliminarArchivo,
	renombrarArchivo,
	moverArchivo,
	crearDirectorio,
	eliminarDirectorio,
	renombrarDirectorio,
	copiarArchivoEnMDFS,
	copiarArchivoDesdeMDFS,
	solicitarMD5,
	verBloque,
	copiarBloque,
	borrarBloque,
	agregarNodo,
	eliminarNodo,*/
	salir,
	enter
};

int main(int number_of_args, char* arg_list[])
{
	int command;
	int i;

	printf("_____________________________________________________________________________________________\n");
	printf("                     Consola VERSION 2.0 | Grupo: Game of Pointers\n");
	printf("_____________________________________________________________________________________________\n");

	printf("\nIngrese el comando deseado o ayuda para conocer los comandos posibles\n");
	command = read(); // read lee la palabra y me devuelve un comando del enum
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
		}
			break;

		case correr:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
		break;

		case finalizar:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
		break;

		case ps:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
		break;

		case cpu:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
		break;

		/*case formatearMDFS:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
			break;
		case eliminarArchivo:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
			break;
		case renombrarArchivo:
		{
			char pathA[LARGO_NOMBRE_PATH] = { NULL };
			char pathN[LARGO_NOMBRE_PATH] = { NULL };
			int lReturn = -1;

			printf("Ingrese el nombre del archivo que desea renombrar\n");
			printf(RUTA_LOCAL);
			scanf("%s", pathA);
			printf("Ingrese el nombre nuevo\n");
			printf(RUTA_LOCAL);
			scanf("%s", pathN);
			lReturn = renombrar(pathA,pathN);
			if (lReturn)
				{
				printf("Hubo un error al intentar renombrar el archivo\n");
				}
				else
				{
				printf("Comando ejecutado correctamente\n");
				}
		}

			break;
		case moverArchivo:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
			break;
		case crearDirectorio:
		{
			char path[LARGO_NOMBRE_PATH] = { NULL };
			int lReturn = -1;

			printf("Ingrese el path de la carpeta que desea crear\n");
			printf("Recuerde que el largo del directorio no puede exceder los %d caracteres\n", LARGO_NOMBRE_PATH);
			printf(RUTA_LOCAL);
			scanf("%s", path);
			lReturn = crearDir(path);
			if (lReturn)
			{
				printf("Hubo un error al intentar crear el directorio\n");
			}
			else
			{
				printf("Comando ejecutado correctamente\n");
			}
		}
			break;
		case eliminarDirectorio:
		{
			char path[LARGO_NOMBRE_PATH] = { NULL };
			int lReturn = -1;

			printf("Ingrese el nombre de la carpeta que desea eliminar\n");
			printf(RUTA_LOCAL);
			scanf("%s", path);
			lReturn = borrarDir(path);
			if (lReturn)
				{
				printf("Hubo un error al intentar borrar el directorio\n");
				}
			else
				{
				printf("Comando ejecutado correctamente\n");
				}
		}
			break;
		case renombrarDirectorio:
		{
			char pathA[LARGO_NOMBRE_PATH] = { NULL };
			char pathN[LARGO_NOMBRE_PATH] = { NULL };
			int lReturn = -1;

			printf("Ingrese el nombre de la carpeta que desea renombrar\n");
			printf(RUTA_LOCAL);
			scanf("%s", pathA);
			printf("Ingrese el nombre nuevo\n");
			printf(RUTA_LOCAL);
			scanf("%s", pathN);
			lReturn = renombrar(pathA,pathN);
			if (lReturn)
				{
				printf("Hubo un error al intentar renombrar el directorio\n");
				}
				else
				{
				printf("Comando ejecutado correctamente\n");
				}
		}
			break;
		case copiarArchivoEnMDFS:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
			break;
		case copiarArchivoDesdeMDFS:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
			break;
		case solicitarMD5:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
			break;
		case verBloque:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
			break;
		case copiarBloque:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
			break;
		case borrarBloque:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
			break;
		case agregarNodo:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
			break;
		case eliminarNodo:
		{
			printf("Este comando todavia no ha sido implemenado\n");
		}
			break;
			//case salir:
			//{
			//	int respuesta = 'n';
			//	printf("Esta seguro que desea salir de la aplicacion? Y:Yes - N:No: ");
			//	respuesta = tolower(getchar());
			//	if(respuesta == 'y')
			//	{
			//		printf("Programa cerrado\n");
			//		//getchar();
			//		return EXIT_SUCCESS;
			//	}
			//}
			//	break;*/
		case enter:
		{
			// no hagas nada, que se refleje el enter en la consola
		}
			break;
		default:
		{
			// No se que tan necesario es esto porque nosotros devolvemos el comando ingresado, pero bueno...
			printf("No se pudo encontrar el comando especificado\n");
			printf("Ingrese el comando deseado o help para conocer los comandos posibles\n");
		}
			break;
		}

		// esto es porque al final de la ejecución de un comando, puede quedar en el flujo estandar el ingreso
		// de algo (generalmente un enter que es lo ultimo que leemos) por teclado, por lo que el autómata interpretaría
		// que se terminó de ingresar un comando y trataría de entenderlo. Para evitar eso al final de cada lectura
		// vaciamos el contenido en el flujo stdin
		fflush(stdin);
		printf("\nIngrese el comando deseado o ayuda para conocer los comandos posibles\n");
		command = read(); // read lee la palabra y me devuelve un comando del enum
	}
}

/****************************************************************
		Lee el comando y lo busca entre los reconocidos
****************************************************************/
int read(void)
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

/****************************************************************
						Crea el directorio
****************************************************************/
int crearDir (char path[LARGO_NOMBRE_PATH])
{
	long lError = 0;
	const char ccEscritura[] = "w+";
	char buff[LARGO_NOMBRE_PATH + 12] = { NULL };
	unsigned int i;


	memcpy(buff, RUTA_LOCAL , 12);
	for (i = 0; !lError; i++)
	{
		if ('/' == path[i] || NULL == path[i])
		{
			memcpy(buff+12, path, i);
			lError = mkdir (buff, ccEscritura);
		}
		if (NULL == path[i] && !lError)
		{
			return EXIT_SUCCESS;
		}
	}

	return EXIT_FAILURE;
}
/*******************************************************************
 	 	 	 	 	 	 borrar directorio
********************************************************************/
int borrarDir (char path[LARGO_NOMBRE_PATH])
{
	long lError = 0;
	/*const char ccEscritura[] = "w+";*/
	char buff[LARGO_NOMBRE_PATH + 12] = { NULL };
	unsigned int i;


	memcpy(buff, RUTA_LOCAL , 12);
	for (i = 0; !lError; i++)
	{
		if ('/' == path[i] || NULL == path[i])
		{
			memcpy(buff+12, path, i);
			lError = rmdir (buff/*, ccEscritura*/);
		}
		if (NULL == path[i] && !lError)
		{
			return EXIT_SUCCESS;
		}
	}

	return EXIT_FAILURE;
}
/*******************************************************************
 	 	 	 	 	 	 renombrar directorio/archivo
********************************************************************/
int renombrar (char pathA[LARGO_NOMBRE_PATH], char pathN[LARGO_NOMBRE_PATH])
{
	long lError = 0;
	/*const char ccEscritura[] = "w+";*/
	char buffA[LARGO_NOMBRE_PATH + 12] = { NULL };
	char buffN[LARGO_NOMBRE_PATH + 12] = { NULL };
	unsigned int i;


	memcpy(buffA, RUTA_LOCAL , 12);
	memcpy(buffN, RUTA_LOCAL , 12);
	for (i = 0; !lError; i++)
	{
		if ('/' == pathA[i] || NULL == pathA[i])
		{
			memcpy(buffA+12, pathA, i);
			memcpy(buffN+12, pathN, i);
			lError = rename(buffA/*, ccEscritura*/, buffN);
		}
		if (NULL == pathA[i] && !lError)
		{
			return EXIT_SUCCESS;
		}
	}

	return EXIT_FAILURE;
}
