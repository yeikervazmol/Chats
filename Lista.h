/**
* @author Karen Troiano		09-10855
* @author Yeiker Vazquez	09-10855
* @grupo  
* 
* Archivo: Lista.h
*
* Descripcion: Contiene las definiciones y librerias utilizadas
* por las listas.
*/

#include <pthread.h>

/**
 * Estructuras utilizadas.
 */

/**
 * @struct Item
 * Tipo de dato para manejar los elementos de las listas.
 * Incluye: 
 * * El nombre del cliente (el cual es unico).
 * * El socket correspondiente al cliente.
 * * Un apuntador a una lista interna.
 * * Un apuntador al siguiente elemento de la lista. 
 */

typedef struct Item {
	char *name;
	int sockfd;
	struct Lista *listaInterna;
	struct Item *ApSig;
	
} Item;

/**
 * @struct Lista
 * Tipo de dato para manejar una lista.
 * Incluye: 
 * * Es un mutex para manejar la lista de forma correta 
 * * 	en  concurrencia.
 * * Un apuntador al primer elemento de la lista. 
 */
typedef struct Lista {
	
	pthread_mutex_t bodyguard;
	Item *primero;
	
} Lista;

/** 
 * Fin de structuras utilizadas.
 */


/** 
 * Definiciones de funciones para lista.
 */

Item *buscar(Lista *cabeza, char *tesoro);
int insertar(Lista *cabeza, char *name, int sockfd);
void eliminar(Lista *cabeza,  Item *sentenciado);
void liberarCompleta(Lista *Completa);
char *listar(Lista *cabeza);

/** 
 *	Fin de  definiciones de funciones para lista.
 */