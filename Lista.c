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

Lista *buscar(Lista *cabeza, char *tesoro)
{
	Lista *aux;
	aux = cabeza;
	int i = 1;
	do {
		i = strcmp(tesoro, aux->name);
		if(i != 0)
		{
			aux = aux->ApSig;
		}
		
	} while((aux != NULL) && (i!=0) );
	return aux;
	
}
	
Lista *insertar(Lista *cabeza, char *name, int sockfd) {
	if (cabeza == 0){
		//La lista no tiene elementos.
		cabeza = (Lista*) malloc(sizeof(Lista));
		if(cabeza != NULL){
			strcpy(cabeza->name,name);
			cabeza->sockfd = sockfd;
			cabeza->listaInterna = NULL;
			cabeza->ApSig = NULL;
			cabeza->ApAnt = NULL;
		} else {
			printf("No se ha podido reservar memoria mediante malloc.\n");
		}
		
		return(cabeza);
	
	} else {
		Lista *aux;
		aux = buscar(cabeza, name);
		if (aux == NULL){
			Lista *nuevo;
			nuevo = (Lista*) malloc (sizeof(Lista));
			if(nuevo!=NULL){
				strcpy(cabeza->name,name);
				cabeza->sockfd = sockfd;
				cabeza->listaInterna = NULL;
				nuevo->ApAnt = NULL;
				nuevo->ApSig = cabeza;
				cabeza->ApAnt = nuevo;
				
				return nuevo;
				
			} else {
				printf("No se ha podido reservar memoria mediante malloc.");
				return cabeza;
			}
		} else {
			printf("%s ya ha sido registrado. Por favor, intente con otro nombre.\n");
			return(cabeza);
		}
			
	}
}

Lista *eliminar(Lista *cabeza, Lista *sentenciado) {
	
	if(cabeza != NULL) {
		Lista *ApSentenciado;
		ApSentenciado = buscar(cabeza, sentenciado->name);
		if(ApSentenciado != NULL) {
			if(ApSentenciado->ApSig == NULL && ApSentenciado->ApAnt == NULL) {
				free(cabeza);
				cabeza = NULL;
			} else if (ApSentenciado->ApSig != NULL && ApSentenciado->ApAnt == NULL) {
				ApSentenciado->ApSig->ApAnt = NULL;
				cabeza = ApSentenciado->ApSig;
				free(ApSentenciado);
			}else if (ApSentenciado->ApSig == NULL && ApSentenciado->ApAnt != NULL) {
				ApSentenciado->ApAnt->ApSig = NULL;
				free(ApSentenciado);
			}else if (ApSentenciado->ApSig != NULL && ApSentenciado->ApAnt != NULL) {
				ApSentenciado->ApAnt->ApSig = ApSentenciado->ApSig;
				ApSentenciado->ApSig->ApAnt = ApSentenciado->ApAnt;
				free(ApSentenciado);
			}
		}
	}
	return cabeza;
}
	
void *liberarCompleta(Lista *Completa)
{
	Lista *aux;
	while(Completa != NULL){
		aux = Completa;
		liberarCompleta(aux->listaInterna);
		Completa = aux->ApSig;
		free(aux);
	}
	return;
}


char *listar(Lista *cabeza) {
	
	Lista *aux;
	aux = cabeza;
	char *elementos = "\n";
	if(aux != NULL) {
		
		while(aux != NULL) {
			strcat(elementos, aux->name);
			strcat(elementos, "\n");
			aux = aux->ApSig;	
		}
	} else {
		printf("La lista no tiene elementos.\n");
	}
	return elementos;
	
}