/*
 * comun.c
 *
 *  Created on: 1/10/2015
 *      Author: utnso
 */


#include "comun.h"



void creoLogger(int pantalla) //recive 0 para log solo x archivo| recive 1 para log x archivo y x pantalla
{
	if (pantalla)
		logger = log_create("log.txt", "Cache13",true, LOG_LEVEL_INFO);
	else
		logger = log_create("log.txt", "Cache13",false, LOG_LEVEL_INFO);

    /*
     * FUNCIONES UTILIZABLES PARA EL LOGGEO, EJEMPLOS:
    log_trace(logger, "LOG A NIVEL %s", "TRACE");
    log_debug(logger, "LOG A NIVEL %s", "DEBUG");
    log_info(logger, "LOG A NIVEL %s", "INFO");
    log_warning(logger, "LOG A NIVEL %s", "WARNING");
    log_error(logger, "LOG A NIVEL %s", "ERROR");
	*/

}

char* cwd(){		// devuelve el current working dir para usar rutas relativas
	char *cwd = getcwd(NULL,0);
return cwd;
}

