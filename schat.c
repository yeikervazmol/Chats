#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <pthread.h>
#include "errors.h"

#define PORT 25503
const int MAXHILOS = 50;
const int CARACTERES = 140;


pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;


int hilosLector[50]; /* arreglo para manejar las lecturas de los hilos.*/
int hilosEnArreglo = 0;
int hilosActivos = 0;
int hilosHanLeido = 0;
int hayMensaje = 0;
char c[140];
int limpiando = 0;

typedef struct ParametrosHilos {
	int id;
	int sockfd;
	int newsockfd;
} ParametrosHilos;


void *getAndWrite(ParametrosHilos *recibe) {
	int i = 0;
	int sockfd = recibe->newsockfd;
	int id = recibe->id;
	for (i=0; i<CARACTERES; i++){
		c[i] = '\0';
	}
	while(1){
		if (hayMensaje == 0){
			if (read(sockfd, c, CARACTERES) > 0) {
				printf("Se identifico la secuencia: %s\n", c);
				hayMensaje = 1;			
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
		
		if ((hayMensaje==1) && (hilosLector[id] == 1) && (limpiando == 0)){
			printf("Guardias: HayMensaje %d hilosLector[%d] %d limpiando %d\n", hayMensaje, id, hilosLector[id], limpiando);
			printf("HayMensaje: %s por el hilo %d.\n", c, id);
			if (write(sockfd, c, CARACTERES) < 0){
				fatalerror("can't write to socket");
			} 
			hilosLector[id] = 0;
			pthread_mutex_lock(&count_mutex);
			hilosHanLeido++;
			pthread_mutex_unlock(&count_mutex);
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

int main(int argc, char *argv []) {

	int sockfd, newsockfd;
	struct sockaddr_in clientaddr, serveraddr;
	int clientaddrlength;
	pthread_t hilos[MAXHILOS]; /* arreglo que contendra todos los hilos.*/
	int i = 0;
	ParametrosHilos envio;

	/* Recuerda el nombre del archivo para mensajes de error. */
	programname = argv[0];
	
	/* Abre el socket TCP. */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		fatalerror("No abre el socket D:");
	}
	/* Bind the address to the socket. */
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);
	if (bind(sockfd, (struct sockaddr *) &serveraddr,
			sizeof(serveraddr)) != 0){
		fatalerror("can't bind to socket");
	}
	
	if (listen(sockfd, MAXHILOS) < 0){
		fatalerror("can't listen");
	}
	
	for (i = 0; i < MAXHILOS; i++) {
		hilosLector[i] = 1;
	}
	
	for (i = 0; i < MAXHILOS; i++) {
		/* Wait for a connection. */
		clientaddrlength = sizeof(clientaddr);
		printf("Esperando nuevo cliente\n");
		newsockfd = accept(sockfd, 
						(struct sockaddr *) &clientaddr,
						&clientaddrlength);
		printf("Cliente Nuevo\n");
		if (newsockfd < 0){
			fatalerror("accept failure");
		}
		envio.id = hilosActivos++;
		hilosEnArreglo++;
		envio.sockfd = sockfd;
		envio.newsockfd = newsockfd;
		if ((pthread_create(&hilos[i], NULL, (void *)echo, (void *)&envio)) != 0){
			fatalerror("Error catastrofico creando hilo :OOO");
		}
	}
	
	for (i = 0; i < MAXHILOS; i++) {
		pthread_join(hilos[i], NULL);
	}
	
	
	printf("\nFin\n");
}