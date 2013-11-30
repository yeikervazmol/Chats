/*
* @author Karen Troiano && Yeiker Vazquez
* @carnet 09-10855 && 09-10855
* @grupo  09
*
* Archivo: Servidor.h
*
* Descripcion: Contiene las definiciones utilizadas por
* el servidor.
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
#include <pthread.h>
#include "errors.h"

/*Estructuras utilizadas */
typedef struct ParametrosHilos {
	int id;
	char *nombreCliente;
	int newsockfd;
} ParametrosHilos;

/* Fin de structuras utilizadas */
