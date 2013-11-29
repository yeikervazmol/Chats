/*
* @author Karen Troiano && Yeiker Vazquez
* @carnet 09-10855 && 09-10855
* @grupo  
*
* Archivo: Lista.c
*
* Descripcion: Contiene el programa principal del
* servidor del chat.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Lista.h"
#include "errors.h"


pthread_t hilos[10];
Lista *miLista;

/* Se debe ser precavido con esta funcion ya que no abre y cierra
 el lock para hilos de la lista ya que es llamada por otras funciones
 que se encargan de eso. Por ello si se usa la funcion directamente
 se deben realizar los lock y unlock antes y despues de usar la
 funcion.*/
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

// PENSAR QUE HACER CON LA LISTA INTERNA.
void eliminar(Lista *lista, Item *sentenciado) {
	if (lista != NULL){
		pthread_mutex_lock(&(lista->bodyguard));
		Item *aux = lista->primero;
		if (aux != NULL){
			// Caso en el que el item a eliminar es el primero
			if (aux == sentenciado){
				lista->primero = aux->ApSig;
				free(aux->name);
				free(aux);
			} else {
				Item *auxAnt;
				while(aux != NULL){
					auxAnt = aux;
					aux = aux->ApSig;
					if (aux == sentenciado){
						auxAnt->ApSig = aux->ApSig;
						free(aux->name);
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
	
void liberarCompleta(Lista *lista)
{
	if (lista != NULL){
		pthread_mutex_lock(&(lista->bodyguard));
		Item *aux = lista->primero;
		Item *auxSig;
		while(aux != NULL){
			// PENSAR BIEN ESTA PARTE
			// liberarCompleta(aux->listaInterna);
			auxSig = aux->ApSig;
			free(aux->name);
			free(aux);
			aux = auxSig;
		}
		pthread_mutex_unlock(&(lista->bodyguard));
		pthread_mutex_destroy(&(lista->bodyguard));
		free(lista);
	}
}


char *listar(Lista *lista) {
	
	char *elementos = calloc(100*40, sizeof(char));
	if(lista != NULL){
		pthread_mutex_lock(&(lista->bodyguard));
		Item *aux;
		aux = lista->primero;
		if(aux != NULL) {
			
			while(aux != NULL) {
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