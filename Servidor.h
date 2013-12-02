/**
* @author Karen Troiano		09-10855
* @author Yeiker Vazquez	09-10855
* @grupo  
*
* Archivo: Servidor.h
*
* Descripcion: Contiene las definiciones y librerias utilizadas
* por el servidor.
*/

#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "errors.h"

/**
 * Estructuras utilizadas
 */

/**
 * @struct ParametrosHilos
 * Tipo de dato para manejar los parametros que son enviados
 *  a las funciones de los respectivos hilos. Incluye: 
 * * El identificador del hilo.
 * * El nombre del cliente (el cual es unico).
 * * El socket correspondiente al cliente.
 */
typedef struct ParametrosHilos {
	int id;
	char *nombreCliente;
	int newsockfd;
} ParametrosHilos;

/** 
 * Fin de structuras utilizadas
 */
