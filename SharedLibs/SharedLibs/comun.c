/*
 * comun.c
 *
 *  Created on: 1/10/2015
 *      Author: utnso
 */


#include "comun.h"

void creoLogger()
{
    logger = log_create("log.txt", "Cache13",true, LOG_LEVEL_INFO);

    /*
     * FUNCIONES UTILIZABLES PARA EL LOGGEO, EJEMPLOS:
    log_trace(logger, "LOG A NIVEL %s", "TRACE");
    log_debug(logger, "LOG A NIVEL %s", "DEBUG");
    log_info(logger, "LOG A NIVEL %s", "INFO");
    log_warning(logger, "LOG A NIVEL %s", "WARNING");
    log_error(logger, "LOG A NIVEL %s", "ERROR");
	*/

}
