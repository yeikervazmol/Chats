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


struct caja {
	
	char *name;
	int sockfd;
	struct caja *listaInterna;
	struct caja *ApSig;
	struct caja *ApAnt;

};

typedef struct caja Lista;


extern Lista *buscar(Lista *cabeza, char *tesoro);
extern Lista *insertar(Lista *cabeza, char *name, int sockfd);
extern Lista *eliminar(Lista *cabeza,  Lista *sentenciado);
extern void *liberarCompleto(Lista *Completa);
extern char *listar(Lista *cabeza);
