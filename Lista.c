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
	
int insertar(Lista *lista, char *name, int sockfd) {
	if (lista != NULL){
		pthread_mutex_lock(&(lista->bodyguard));
		Item *cabeza = lista->primero;
		if (cabeza == NULL){
			//La lista no tiene elementos.
			cabeza = calloc(1, sizeof(Item));
			printf("Estoy haciendo calloc \n");
			if(cabeza != NULL){
				printf("Hice calloc y tengo memoria\n");
				cabeza->name = (char *) calloc(strlen(name)+1, sizeof(char));
				printf("Hago calloc del nombre...");
				if (cabeza->name != NULL){
					printf("Hice calloc y voy a insertar");
					lista->primero = cabeza;
					printf("Lista->primero = cabeza \n");
					strcpy(cabeza->name,name);
					printf("Lista->primero = cabeza1\n");
					cabeza->sockfd = sockfd;
					printf("Lista->primero = cabeza2 \n");
					cabeza->listaInterna = NULL;
					printf("Lista->primero = cabeza 3\n");
					cabeza->ApSig = NULL;
					printf("Lista->primero = cabeza 4\n");
				} else {
					printf("Hice calloc y voy a hacer free\n");
					free(cabeza);
					lista->primero = NULL;
					return 0;
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
					exit(1);
				}
			} else {
				return 0;
			}
				
		}
		return 1;
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

/*void escribir(int *j){
		Item *sentenciado;
		int i = *j;
		printf("Este es el hilo %d comenzando\n", i+1);
		char *m1;
		char *m2;
		char *m3;
		char *m4;
		char *m5;
		
		if (i == 0){
			m1 = "Uno there";
			m2 = "Uno there uno";
			m3 = "Uno there dos";
			m4 = "Uno there tres";
			m5 = "Uno there cuatro";
		}
		
		if (i == 1){
			m1 = "Dos there";
			m2 = "Dos there uno";
			m3 = "Dos there dos";
			m4 = "Dos there tres";
			m5 = "Dos there cuatro";
		}
		
		if (i == 2){
			m1 = "Tres there";
			m2 = "Tres there uno";
			m3 = "Tres there dos";
			m4 = "Tres there tres";
			m5 = "Tres there cuatro";
		}
		
		if (i == 3){
			m1 = "Cuatro there";
			m2 = "Cuatro there uno";
			m3 = "Cuatro there dos";
			m4 = "Cuatro there tres";
			m5 = "Cuatro there cuatro";
		}
		
		if (i == 4){
			m1 = "Quinto there";
			m2 = "Quinto there uno";
			m3 = "Quinto there dos";
			m4 = "Quinto there tres";
			m5 = "Quinto there cuatro";
		}
		
		insertar(miLista, m1, 3);
		insertar(miLista, m2, 3);
		insertar(miLista, m3, 3);
		insertar(miLista, m4, 3);
		insertar(miLista, m5, 3);
		
		printf("Elementos despues de insertar por el hilo %d:\n%s", i+1, listar(miLista));
		
		pthread_mutex_lock(&(miLista->bodyguard));
		sentenciado = buscar(miLista, m1);
		pthread_mutex_unlock(&(miLista->bodyguard));
		eliminar(miLista, sentenciado);
		
		pthread_mutex_lock(&(miLista->bodyguard));
		sentenciado = buscar(miLista, m2);
		pthread_mutex_unlock(&(miLista->bodyguard));
		eliminar(miLista, sentenciado);
		
		pthread_mutex_lock(&(miLista->bodyguard));
		sentenciado = buscar(miLista, m3);
		pthread_mutex_unlock(&(miLista->bodyguard));
		eliminar(miLista, sentenciado);
		
		pthread_mutex_lock(&(miLista->bodyguard));
		sentenciado = buscar(miLista, m4);
		pthread_mutex_unlock(&(miLista->bodyguard));
		eliminar(miLista, sentenciado);
		
		pthread_mutex_lock(&(miLista->bodyguard));
		sentenciado = buscar(miLista, m5);
		pthread_mutex_unlock(&(miLista->bodyguard));
		eliminar(miLista, sentenciado);
		
		return;
		
		
	}

int main(int argc, char *argv []) {

	miLista = calloc(1, sizeof(Lista));
	int argumentos[5];
	if (pthread_mutex_init(&(miLista->bodyguard), NULL) != 0)
	{
		free(miLista);
		return 0;
	}
	int i;
	for (i = 0; i < 5; i++) {
		argumentos[i] = i;
		if ((pthread_create(&hilos[i], NULL, (void *)escribir, &argumentos[i])) != 0){
			printf("Error\n");
		}
	}
	
	for (i = 0; i < 5; i++) {
		pthread_join(hilos[i], NULL);
	}
	char *contenido = listar(miLista);
	printf("Estos son los elementos:\n%s", contenido);
	liberarCompleta(miLista);
	free(contenido);
	
	return 0;
}*/	
