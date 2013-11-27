/*
* @author Karen Troiano && Yeiker Vazquez
* @carnet 09-10855 && 09-10855
* @grupo  09
*
* Archivo: Servidor.c
*
* Descripcion: Contiene las funciones utilizadas 
* por el servidor.
*/

#include "Servidor.h"

const int MAXHILOS = 50;
const int CARACTERES = 140;

pthread_mutex_t count_mutex;

int hilosLector[50]; /* arreglo para manejar las lecturas de los hilos.*/
int hilosEnArreglo = 0;
int hilosActivos = 0;
int hilosHanLeido = 0;
int hayMensaje = 0;
char c[140];
int limpiando = 0;
int puerto = 25504;
char *salas[100];


void *getAndWrite(ParametrosHilos *recibe) {
	int i = 0;
	int sockfd = recibe->newsockfd;
	int id = recibe->id;
	
	for (i=0; i<CARACTERES; i++){
		c[i] = '\0';
	}
	if (hilosLector[id] == 2){
		hilosLector[id] = 1;
	}
	while(1){
		if (hayMensaje == 0){
			if (read(sockfd, c, CARACTERES) > 0) {
				printf("Se identifico la secuencia: %s en el hilo %d\n", c, id);
				pthread_mutex_lock(&count_mutex);
				hayMensaje = 1;	
				pthread_mutex_unlock(&count_mutex);
			} else {
				hilosLector[id] = 2;
				hilosActivos--;
				close(sockfd);
				break;
			}
		}
	}
}

void *readAndPrint(ParametrosHilos *recibe){
        int sockfd = recibe->newsockfd;
        int id = recibe->id;
        int i;
        while(1){
				if (hayMensaje == 1) {
					if ((hayMensaje==1) && (hilosLector[id] == 1) && (limpiando == 0)){
						printf("Guardias: HayMensaje %d hilosLector[%d] %d limpiando %d\n", hayMensaje, id, hilosLector[id], limpiando);
						printf("Guardia HayMensaje == 1: %d\n", (hayMensaje==1));
						printf("HayMensaje: %s por el hilo %d.\n", c, id);
						if (write(sockfd, c, CARACTERES) < 0){
							fatalerror("can't write to socket");
						} 
						pthread_mutex_lock(&count_mutex);
						hilosLector[id] = 0;
						hilosHanLeido++;
						printf("HiloHanLeido %d por el hilo %d.\n", hilosHanLeido, id);
						if (hilosHanLeido == hilosActivos){
							hayMensaje = 0;
							limpiando = 1;
							for (i=0;i<hilosEnArreglo;i++){
									if (hilosLector[i] == 0) {
											hilosLector[i] = 1;
									}        
							}
							hilosHanLeido = 0;
							limpiando = 0;
							
						}
						pthread_mutex_unlock(&count_mutex);
					}
                }
                
                if (hilosLector[id] == 2){
						close(sockfd);
                        break;
                }
                
                
        }
        
}


void *echo(ParametrosHilos *recibe) {
	pthread_t hiloW, hiloR;
	pthread_create(&hiloW, NULL, (void *)getAndWrite, recibe);
	pthread_create(&hiloR, NULL, (void *)readAndPrint, recibe);
	pthread_join(hiloW, NULL);
	pthread_join(hiloR, NULL);
	return NULL;
}

