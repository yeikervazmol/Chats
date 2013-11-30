/*
* @author Karen Troiano && Yeiker Vazquez
* @carnet 09-10855 && 09-10855
* @grupo  09
*
* Archivo: schat.c
*
* Descripcion: Contiene el programa principal del
* servidor del chat.
*/

#include "Servidor.h"
#include "Lista.h"

#define BUFFERTAM 1024

const int MAXHILOS = 50;
const int CARACTERES = 140;

pthread_mutex_t count_mutex;
pthread_mutex_t _mutex;

int hilosLector[50]; /* arreglo para manejar las lecturas de los hilos.*/
int hilosEnArreglo = 0;
int hilosActivos = 0;
int hilosHanLeido = 0;
int hayMensaje = 0;
char c[140];
int limpiando = 0;
int puerto = 25504;
Lista *clientes;
Lista *salas;
char *nombreSala;

char *suscribirSala(char *salaSus, char *nombreCliente, int sockfd){
	
	Item *item;
	
	pthread_mutex_lock(&(salas->bodyguard));
	item = buscar(salas, salaSus);
	pthread_mutex_unlock(&(salas->bodyguard));
	
	
	if(item == NULL){
		return "0";
	}
	
	if( insertar(item->listaInterna, nombreCliente, sockfd) == 0){
		return "0";
	}
	
	printf("Lista de usuarios de la sala %s:\n%s\n", item->name, listar(item->listaInterna));
	
	pthread_mutex_lock(&(clientes->bodyguard));
	item = buscar(clientes, nombreCliente);
	pthread_mutex_unlock(&(clientes->bodyguard));
	
	if(item == NULL){
		return "0";
	}
	
	if( insertar(item->listaInterna, salaSus, sockfd) == 0){
		return "0";
	}
	
	printf("Lista de salas del usuario %s:\n%s\n", item->name, listar(item->listaInterna));
	
	return "1";
	
}

char *desuscribirSala(char *nombreCliente){
	Item *cliente;
	Item *aux;
	Item *sala;
	Item *sentenciado;
	Item *ant;

	pthread_mutex_lock(&(clientes->bodyguard));
	cliente = buscar(clientes, nombreCliente);
	pthread_mutex_unlock(&(clientes->bodyguard));
	
	aux = cliente->listaInterna->primero;

	while (aux != NULL) {
		
		pthread_mutex_lock(&(salas->bodyguard));
		sala = buscar(salas, aux->name);
		pthread_mutex_unlock(&(salas->bodyguard));
	
		pthread_mutex_lock(&(sala->listaInterna->bodyguard));
		sentenciado = buscar(sala->listaInterna, nombreCliente);
		pthread_mutex_unlock(&(sala->listaInterna->bodyguard));
	
		eliminar(sala->listaInterna, sentenciado);
		ant = aux;
		aux = aux->ApSig;
		eliminar(cliente->listaInterna,ant);
	}
	
	//liberarCompleta(cliente->listaInterna);
	return "1";
}

void *atencionCliente(ParametrosHilos *recibe){
	
	char *comando = calloc(BUFFERTAM, sizeof(char));
	char *respuesta = calloc(BUFFERTAM, sizeof(char));
	int i;
	Item *item;
	while(1){
		if (read(recibe->newsockfd, comando, BUFFERTAM) < 0) {
			fatalerror("can't read the socket");
		}
		
		switch (comando[0]) {
			case 's':
				if(comando[1] == 'a'){
					respuesta = listar(salas);
				} else {
					respuesta = suscribirSala(comando+4, recibe->nombreCliente, recibe->newsockfd);
				}
				break;
			case 'u': 
				respuesta = listar(clientes);
				break;
			case 'm': 
				break;
			case 'd': 
				respuesta = desuscribirSala(recibe->nombreCliente);
				break;
			case 'c':
				i = insertar(salas, comando + 4, 0);

				if (i == 1) {
					pthread_mutex_lock(&(salas->bodyguard));
					item = buscar(salas, comando + 4);
					pthread_mutex_unlock(&(salas->bodyguard));
					item->listaInterna = calloc(1, sizeof(Lista));
					if(item->listaInterna == NULL){
						fatalerror("No se puede alocar memoria para una lista interna.\n");
					}
					if (pthread_mutex_init(&(item->listaInterna->bodyguard), NULL) != 0) {
						fatalerror("No se puede inicializar el mutex de una lista.\n");
					}
					respuesta = "1";
				} else {
					respuesta = "0";
				}
				break;
			case 'e': 
				pthread_mutex_lock(&(salas->bodyguard));
				item = buscar(salas, comando + 4);
				pthread_mutex_unlock(&(salas->bodyguard));
				if (item == NULL) {
					free(item);
					respuesta = "0";
				} else {
					eliminar(salas, item);
					respuesta = "1";
				}
				break;
			case 'f': 
				break;
			default:
				printf("Error de protocolo con el cliente\n");
				break;
		}
		
		if (write(recibe->newsockfd, respuesta, BUFFERTAM) < 0){
			fatalerror("can't write to socket");
		} 
	}
	
}

/*
void *readAndPrint(ParametrosHilos *recibe){
	
        int id = recibe->id;
        int i;
        while(1) {
				if ((hayMensaje==1) && (hilosLector[id] == 1) && (limpiando == 0)){
					//printf("Guardias: HayMensaje %d hilosLector[%d] %d limpiando %d\n", hayMensaje, id, hilosLector[id], limpiando);
					//printf("Guardia HayMensaje == 1: %d\n", (hayMensaje==1));
					//printf("HayMensaje: %s por el hilo %d.\n", c, id);
					if (write(sockfd, c, CARACTERES) < 0){
						fatalerror("can't write to socket");
					} 
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
				}
				
				if (hilosLector[id] == 2){
						close(sockfd);
						break;
				}
	}
}
*/
void *echo(ParametrosHilos *recibe) {
	
	ParametrosHilos *recibe2 = calloc(1, sizeof(ParametrosHilos));
	recibe2->newsockfd = recibe->newsockfd;
	recibe2->id = recibe->id;
	int nombreValido = 0;
	char *nombreCliente = calloc(BUFFERTAM, sizeof(char));
	Item *salaPredeterminada;
	Item *clienteActual;
	
	if (recibe2 == NULL){
		fatalerror("No se ha podido reservar memoria mediante calloc.\n");
	}
	
	while(nombreValido == 0	){
		if (read(recibe2->newsockfd, nombreCliente, BUFFERTAM) < 0) {
			fatalerror("can't listen to socket");
		}
		nombreValido = insertar(clientes, nombreCliente,recibe2->newsockfd);
		if( nombreValido ){
			
			pthread_mutex_lock(&(clientes->bodyguard));
			clienteActual = buscar(clientes, nombreCliente);
			pthread_mutex_unlock(&(clientes->bodyguard));
			
			clienteActual->listaInterna = calloc(1, sizeof(Lista));
			if( clienteActual->listaInterna == NULL){
				liberarCompleta(clientes);
				fatalerror("No se ha podido reservar memoria mediante calloc.\n");
			} else {
				if (pthread_mutex_init(&(clienteActual->listaInterna->bodyguard), NULL) != 0) {
					free(clienteActual->listaInterna);
					liberarCompleta(clientes);
					fatalerror("No se ha podido inicializar un mutex.\n");
				}
			}
		}
		
		if (write(recibe2->newsockfd, &nombreValido, sizeof(int)) < 0){
			fatalerror("can't write to socket");
		}
	}
	
	recibe2->nombreCliente = nombreCliente;
	
	pthread_mutex_lock(&(salas->bodyguard));
	salaPredeterminada = buscar(salas, nombreSala);
	pthread_mutex_unlock(&(salas->bodyguard));
	if( insertar((salaPredeterminada->listaInterna), nombreCliente, recibe2->newsockfd) == 0 ){
		fatalerror("No se pudo asociar un cliente a su sala.\n");
	}
	
	pthread_mutex_lock(&(clientes->bodyguard));
	clienteActual = buscar(clientes, nombreCliente);
	pthread_mutex_unlock(&(clientes->bodyguard));
	if( insertar((clienteActual->listaInterna), nombreSala, 0) == 0 ){
		fatalerror("No se pudo asociar una sala a un cliente.\n");
	}
	printf("Lista de salas del usuario %s\n%s", clienteActual->name, listar(clienteActual->listaInterna));
	
	
	
	pthread_t hiloW, hiloR;
	pthread_create(&hiloW, NULL, (void *)atencionCliente, recibe2);
//	pthread_create(&hiloR, NULL, (void *)readAndPrint, recibe2);
	pthread_join(hiloW, NULL);
//	pthread_join(hiloR, NULL);
	
	free(recibe2);
	free(nombreCliente);
	return NULL;
}



int main(int argc, char *argv []) {	
	int sockfd, newsockfd;
	struct sockaddr_in clientaddr, serveraddr;
	int clientaddrlength;
	pthread_t hilos[MAXHILOS]; /* arreglo que contendra todos los hilos.*/
	int i = 0;
	ParametrosHilos envio;
	pthread_mutex_init(&count_mutex, NULL);
	pthread_mutex_init(&_mutex, NULL);
	char key;
	nombreSala = calloc(BUFFERTAM+1, sizeof(char));
	nombreSala = "actual";
	
	/* Se inicializa la sala de los clientes */
	clientes = calloc(1, sizeof(Lista));
	if (clientes == NULL){
		printf("Error");
		exit(1);
	}
	if (pthread_mutex_init(&(clientes->bodyguard), NULL) != 0) {
		free(clientes);
		return 0;
	}
	
	/* Se inicializa la lista de salas */
	salas = calloc(1, sizeof(Lista));
	if (salas == NULL){
		printf("Error");
		exit(1);
	}
	if (pthread_mutex_init(&(salas->bodyguard), NULL) != 0) {
		free(salas);
		return 0;
	}
	
	
	/* Recuerda el nombre del archivo para mensajes de error. */
	programname = argv[0];
	
	/* Verificamos los parametros de entrada */
	while ((key = getopt(argc, argv, "p:s:")) != -1) {
		switch (key) {
			case 'p':
				puerto = atoi(optarg);
				break;
			case 's':
				nombreSala = optarg;
				break;
			default:
				printf("Forma incorrecta de invocacion del programa, mejor intete: schat [-p <puerto>] [-s <sala>]\n");
				exit(EXIT_FAILURE);
				break;
			}
	}
	
	/* Agregamos la sala principal del chat */
	if(insertar(salas, nombreSala, 0)){
		salas->primero->listaInterna = calloc(1, sizeof(Lista));
		if( salas->primero->listaInterna == NULL){
			liberarCompleta(salas);
			fatalerror("No se ha podido reservar memoria mediante calloc.\n");
		} else {
			if (pthread_mutex_init(&(salas->primero->listaInterna->bodyguard), NULL) != 0) {
				free(salas->primero->listaInterna);
				liberarCompleta(salas);
				fatalerror("No se ha podido inicializar un mutex.\n");
			}
		}
	}
	
	
	/* Abre el socket TCP. */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		fatalerror("No abre el socket D:");
	}
	/* Bind the address to the socket. */
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(puerto);
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
		
		envio.id = hilosEnArreglo++;
		printf("hilosActivos %d\n",hilosEnArreglo);
		hilosActivos++;
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
