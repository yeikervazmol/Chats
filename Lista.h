/*
* @author Karen Troiano && Yeiker Vazquez
* @carnet 09-10855 && 09-10855
* @grupo  
*
* Archivo: Lista.h
*
* Descripcion: Contiene el programa principal del
* servidor del chat.
*/

#include <pthread.h>
	
typedef struct Item {
	
	char *name;
	int sockfd;
	struct Lista *listaInterna;
	struct Item *ApSig;
	
} Item;

typedef struct Lista {
	
	pthread_mutex_t bodyguard;
	Item *primero;
	
} Lista;


Item *buscar(Lista *cabeza, char *tesoro);
int insertar(Lista *cabeza, char *name, int sockfd);
void eliminar(Lista *cabeza,  Item *sentenciado);
void liberarCompleta(Lista *Completa);
char *listar(Lista *cabeza);
