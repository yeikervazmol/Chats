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
Item *buscar(Lista *lista, char *tesoro)
{
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
	
void insertar(Lista *lista, char *name, int sockfd) {
	if (lista != NULL){
		pthread_mutex_lock(&(lista->bodyguard));
		Item *cabeza = lista->primero;
		if (cabeza == NULL){
			//La lista no tiene elementos.
			cabeza = calloc(1, sizeof(Item));
			if(cabeza != NULL){
				cabeza->name = calloc(strlen(name)+1, sizeof(char));
				if (cabeza->name != 0){
					lista->primero = cabeza;
					strcpy(cabeza->name,name);
					cabeza->sockfd = sockfd;
					cabeza->listaInterna = NULL;
					cabeza->ApSig = NULL;
				} else {
					free(cabeza);
					lista->primero = NULL;
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
					printf("No se ha podido reservar memoria mediante calloc.");
				}
			} else {
				printf("\n%s ya ha sido registrado. Por favor, intente con otro nombre.\n", name);
			}
				
		}
		pthread_mutex_unlock(&(lista->bodyguard));	
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

void escribir(int i){
		Item *sentenciado;
		insertar(miLista, "1", 3);
		insertar(miLista, "2", 3);
		insertar(miLista, "3", 3);
		insertar(miLista, "4", 3);
		insertar(miLista, "5", 3);
		
		pthread_mutex_lock(&(miLista->bodyguard));
		sentenciado = buscar(miLista, "1");
		pthread_mutex_unlock(&(miLista->bodyguard));
		eliminar(miLista, sentenciado);
		
		pthread_mutex_lock(&(miLista->bodyguard));
		sentenciado = buscar(miLista, "2");
		pthread_mutex_unlock(&(miLista->bodyguard));
		eliminar(miLista, sentenciado);
		
		pthread_mutex_lock(&(miLista->bodyguard));
		sentenciado = buscar(miLista, "3");
		pthread_mutex_unlock(&(miLista->bodyguard));
		eliminar(miLista, sentenciado);
		
		pthread_mutex_lock(&(miLista->bodyguard));
		sentenciado = buscar(miLista, "4");
		pthread_mutex_unlock(&(miLista->bodyguard));
		eliminar(miLista, sentenciado);
		
		pthread_mutex_lock(&(miLista->bodyguard));
		sentenciado = buscar(miLista, "5");
		pthread_mutex_unlock(&(miLista->bodyguard));
		eliminar(miLista, sentenciado);
		
		return;
		
		
	}

int main(int argc, char *argv []) {

	miLista = calloc(1, sizeof(Lista));
	if (pthread_mutex_init(&(miLista->bodyguard), NULL) != 0)
	{
		free(miLista);
		return 0;
	}
	int i;
	for (i = 0; i < 10; i++) {
		if ((pthread_create(&hilos[i], NULL, (void *)escribir, &i)) != 0){
			printf("Error\n");
		}
	}
	
	char *contenido = listar(miLista);
	printf("Estos son los elementos:\n%s", contenido);
	liberarCompleta(miLista);
	free(contenido);
	
	return 0;
}	