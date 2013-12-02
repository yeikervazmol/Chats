/**
* @author Karen Troiano		09-10855
* @author Yeiker Vazquez	09-10855
* @grupo  
*
* Archivo: Lista.c
*
* Descripcion: Contiene las funciones necesarias para
* el control las estructuras de listas.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Lista.h"
#include "errors.h"

/**
 * Funcion encargada de la busqueda de un elemento en la lista.
 * 
 * @param La lista en la cual sera buscado el elemento.
 * @param El elemento a ser buscado.
 * @return Devuelve el item del objeto buscado.
 * 
 * * Esta funcion no se encuentra protegida por los mutex.
 * 
 */
Item *buscar(Lista *lista, char *tesoro) {
	if(lista != NULL) {
		Item *aux;
		aux = lista->primero;
		while (aux != NULL){
			if ( !(strcmp(tesoro, aux->name)) ){
				pthread_mutex_unlock(&(lista->bodyguard));
				return aux;
			} else {
				aux = aux->ApSig;
			}	
		}
	}
	return NULL;
}

/**
 * Funcion encargada de insertar un elemento en la lista.
 * 
 * @param La lista en la cual sera insertado el elemento.
 * @param El nombre del elemento a ser insertado.
 * @param El numero del socket del cliente. En caso de ser
 *	una sala, se recibe 0.
 * @return Devuelve 1 si inserta con exito, 0 en caso contrario.
 * 
 * * Esta funcion se encuentra protegida por los mutex.
 * 
 */
int insertar(Lista *lista, char *name, int sockfd) {
	if (lista != NULL){
		pthread_mutex_lock(&(lista->bodyguard));
		Item *cabeza = lista->primero;
		if (cabeza == NULL){
			//La lista no tiene elementos.
			cabeza = calloc(1, sizeof(Item));
			if(cabeza != NULL){
				cabeza->name = calloc(strlen(name)+1, sizeof(char));
				if (cabeza->name != NULL){
					lista->primero = cabeza;
					strcpy(cabeza->name,name);
					cabeza->sockfd = sockfd;
					cabeza->listaInterna = NULL;
					cabeza->ApSig = NULL;
				} else {
					free(cabeza);
					lista->primero = NULL;
					printf("No se ha podido reservar memoria mediante calloc.");
					exit(1);
				}
				
			}
		
		} else {
			Item *aux;
			aux = buscar(lista, name);
			if (aux == NULL){
				Item *nuevo;
				nuevo = calloc (1, sizeof(Item));
				if(nuevo != NULL){
					nuevo->name = calloc(strlen(name)+1, sizeof(char));
					strcpy(nuevo->name, name);
					nuevo->listaInterna = NULL;
					nuevo->sockfd = sockfd;
					nuevo->ApSig = cabeza;
					lista->primero = nuevo;
					
				} else {
					free(nuevo);
					printf("No se ha podido reservar memoria mediante calloc.");
					exit(1);
				}
			} else {
				pthread_mutex_unlock(&(lista->bodyguard));	
				return 0;
			}
				
		}
		pthread_mutex_unlock(&(lista->bodyguard));	
		return 1;
	}
}

/**
 * Funcion encargada de liberar la memoria reservada por la lista.
 * 
 * @param La lista en la cual sera destruida.
 * 
 * * Esta funcion se encuentra protegida por los mutex.
 * 
 */
void liberarCompleta(Lista *lista) {
	if (lista != NULL){
		pthread_mutex_lock(&(lista->bodyguard));
		Item *aux = lista->primero;
		Item *auxSig;
		while(aux != NULL){
			auxSig = aux->ApSig;
			free(aux->name);
			liberarCompleta(aux->listaInterna);
			free(aux);
			aux = auxSig;
		}
		pthread_mutex_unlock(&(lista->bodyguard));
		pthread_mutex_destroy(&(lista->bodyguard));
		//free(lista);
	}
}


/**
 * Funcion encargada eliminar el objeto especifico.
 * 
 * @param La lista en la cual se eliminara el elemento.
 * @param El item el cual sera eliminado.
 * 
 * * Esta funcion se encuentra protegida por los mutex.
 * 
 */
void eliminar(Lista *lista, Item *sentenciado) {
	if (lista != NULL){
		pthread_mutex_lock(&(lista->bodyguard));
		Item *aux = lista->primero;
		if (aux != NULL){
			// Caso en el que el item a eliminar es el primero
			if (aux == sentenciado){
				lista->primero = aux->ApSig;
				free(aux->name);
				liberarCompleta(aux->listaInterna);
				free(aux);
			} else {
				Item *auxAnt;
				while(aux != NULL){
					auxAnt = aux;
					aux = aux->ApSig;
					if (aux == sentenciado){
						auxAnt->ApSig = aux->ApSig;
						free(aux->name);
						liberarCompleta(aux->listaInterna);
						free(aux);
						pthread_mutex_unlock(&(lista->bodyguard));	
						return;
					}
				}				
			}
		}
		pthread_mutex_unlock(&(lista->bodyguard));	
	}

}


/**
 * Funcion encargada de listar los nombres de los elementos 
 * 	de una lista.
 * 
 * @param La lista en la cual sera impresa.
 * @return Un string con todos los nombres de los elementos de
 * 	la lista.
 * 
 * * Esta funcion se encuentra protegida por los mutex.
 * 
 */
char *listar(Lista *lista) {
	
	char *elementos = calloc(100*40, sizeof(char));
	if(lista != NULL){
		pthread_mutex_lock(&(lista->bodyguard));
		Item *aux;
		aux = lista->primero;
		if(aux != NULL) {
			
			while(aux != NULL) {
				strcat(elementos, "\t");
				strcat(elementos, aux->name);
				strcat(elementos, "\n");
				aux = aux->ApSig;	
			}
		} else {
			printf("La lista no tiene elementos.\n");
		}
		pthread_mutex_unlock(&(lista->bodyguard));
		
	}
	return elementos;
}