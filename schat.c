/**
* @author Karen Troiano		09-10855
* @author Yeiker Vazquez	09-10855
* @grupo  
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

/**
 * Variables globales que seran compartidas por los hilos.
 */

/* Cantidad de hilos en el arrglo. */
int hilosEnArreglo = 0; 
/* Numero del puerto de la conecxion por defecto */
int puerto = 25504;
/* Listas de salas y clientes activos. */
Lista *clientes;
Lista *salas;
char *nombreSala;
/* 
 * Variable para saber si el servidor ha abortado 
 *	abrutamente el programa.
 */
int abortar = 0;
/* Variable del socket del servidor */
int sockfd;
pthread_t hilos[50]; /* arreglo que contendra todos los hilos.*/

/**
 * Fin de las variables globales.
 */

/**
 * 
 * Funcion encargada de abortar ejecucion si el
 * 	servidor envia una interrupcion al programa
 * 	durante su ejecucion.
 * 
 */
void abortarSeguro(){
	Item *cliente;
	int i;
	abortar = 1;
	
	cliente = clientes->primero;
	
	while (cliente != NULL) {
		if (write(cliente->sockfd,"fue", BUFFERTAM) < 0){
				fatalerror("can't write to socket");
		}
		cliente = cliente->ApSig;
	}
	
	printf("\nTus clientes han sido advertidos acerca de esta interrupcion.\n");
	
	for (i = 0; i < hilosEnArreglo; i++) {
		pthread_join(hilos[i], NULL);
	}
	
	liberarCompleta(salas);
	liberarCompleta(clientes);
	close(sockfd);
	exit(1);
}

/**
 * Funcion encargada de inicializar al cliente al entrar
 * 	por primera vez al servidor.
 * 
 * @param Estructura con el id del hilo, el nombre del cliente y 
 * 	su correspondido socket.
 * 
 */
void *inicializarCliente(ParametrosHilos *recibe){
	
	int nombreValido = 0;
	char *nombreCliente = calloc(BUFFERTAM, sizeof(char));
	Item *salaPredeterminada;
	Item *clienteActual;
	
	if (nombreCliente == NULL) {
		fatalerror("Error reservando memoria mediante calloc");
	}
	
	/* Se obliga a tener un nombre unico de usuario */
	while(nombreValido == 0	){
		if (read(recibe->newsockfd, nombreCliente, BUFFERTAM) < 0) {
			fatalerror("can't listen to socket");
		}
		nombreValido = insertar(clientes, nombreCliente,recibe->newsockfd);
		
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
		
		if (write(recibe->newsockfd, &nombreValido, sizeof(int)) < 0){
			fatalerror("can't write to socket");
		}
	}
	
	recibe->nombreCliente = nombreCliente;
	
	/* Se inserta el nuevo cliente a la sala predeterminada */
	pthread_mutex_lock(&(salas->bodyguard));
	salaPredeterminada = buscar(salas, nombreSala);
	pthread_mutex_unlock(&(salas->bodyguard));
	if( insertar((salaPredeterminada->listaInterna), nombreCliente, recibe->newsockfd) == 0 ){
		fatalerror("No se pudo asociar un cliente a su sala.\n");
	}
	
	/**
	 * Se inserta el nuevo cliente a la lista de clientes activos y 
	 * se le agrega en su lista interna que pertenece a la sala 
	 * predeterminada
	 */
	pthread_mutex_lock(&(clientes->bodyguard));
	clienteActual = buscar(clientes, nombreCliente);
	pthread_mutex_unlock(&(clientes->bodyguard));
	
	if( insertar((clienteActual->listaInterna), nombreSala, 0) == 0 ){
		fatalerror("No se pudo asociar una sala a un cliente.\n");
	}
	
	return;
}

/**
 * Funcion encargada de suscribir un cliente a una sala.
 * 
 * @param Nombre de la sala a la cual sera insertado el usuario.
 * @param El nombre del usuario a ser insertado.
 * @param El numero del socket del usuario a ser insertado.
 * @return String con la respuesta del servidor al cliente.
 * 
 */
char *suscribirSala(char *salaSus, char *nombreCliente, int sockfd){
	
	Item *item;
	
	/* Se agrega en la lista interna de las salas el cliente. */
	pthread_mutex_lock(&(salas->bodyguard));
	item = buscar(salas, salaSus);
	pthread_mutex_unlock(&(salas->bodyguard));
	
	
	if(item == NULL){
		return "Lo sentimos, no puede suscribirse a dicha sala pues la misma no existe.\n";
	}
	
	if( insertar(item->listaInterna, nombreCliente, sockfd) == 0){
		return "Lo sentimos, no puedo suscribirse a dicha sala por problemas en el servidor.\n";
	}
	
	/* Se agrega en la lista interna del cliente la sala. */
	pthread_mutex_lock(&(clientes->bodyguard));
	item = buscar(clientes, nombreCliente);
	pthread_mutex_unlock(&(clientes->bodyguard));
	
	if(item == NULL){
		return "Lo sentimos, no puedo suscribirse a dicha sala por problemas en el servidor.\n";
	}
	
	if( insertar(item->listaInterna, salaSus, sockfd) == 0){
		return "Lo sentimos, no puedo suscribirse a dicha sala por problemas en el servidor.\n";
	}
	
	return "Usted se ha sido suscrito a la sala con exito.\n";	
}

/**
 * Funcion encargada de enviar un mensaje a los clientes.
 * 
 * @param Mensaje a ser enviado.
 * @param El nombre del usuario quien envia el mensaje.
 * @return String con la respuesta del servidor al cliente.
 * 
 */
char *enviarMensaje(char *mensaje,char *nombreUsuario){
	Item *cliente;
	Item *aux;
	Item *sala;
	Item *usuarioDestino;
	char *mensajeBonito = calloc(BUFFERTAM, sizeof(char));
	
	if (mensajeBonito == NULL) {
		fatalerror("Error reservando memoria mediante calloc");
	}
	
	/**
	 * Se busca el cliente que envio el mensaje
	 *	se busca en su lista interna las salas a las
	 * 	que pertenece. Se envia el mensaje a las
	 * 	personas que estan suscritas a las salas que
	 * 	pertenece el mismo usuario.
	 */
	pthread_mutex_lock(&(clientes->bodyguard));
	cliente = buscar(clientes, nombreUsuario);
	pthread_mutex_unlock(&(clientes->bodyguard));
	
	aux = cliente->listaInterna->primero;
	
	while (aux != NULL) {
		
		pthread_mutex_lock(&(salas->bodyguard));
		sala = buscar(salas, aux->name);
		pthread_mutex_unlock(&(salas->bodyguard));
		
		usuarioDestino = sala->listaInterna->primero;
		while (usuarioDestino != NULL) {
			strcpy(mensajeBonito, "En sala ");
			strcat(mensajeBonito, aux->name);
			strcat(mensajeBonito, ", ");
			strcat(mensajeBonito, nombreUsuario);
			strcat(mensajeBonito, " dice: ");
			strcat(mensajeBonito, mensaje);
			strcat(mensajeBonito, "\n");
			
			if (write(usuarioDestino->sockfd, mensajeBonito, BUFFERTAM) < 0){
				fatalerror("can't write to socket");
			}
			usuarioDestino = usuarioDestino->ApSig;
		}
	
		aux = aux->ApSig;
	}
	
	free(mensajeBonito);
	return "";
}

/**
 * Funcion encargada de desuscribir un cliente a todas las salas.
 * 
 * @param El nombre del usuario a ser desuscrito.
 * @return String con la respuesta del servidor al cliente.
 * 
 */
char *desuscribirSala(char *nombreCliente){
	Item *cliente;
	Item *aux;
	Item *sala;
	Item *sentenciado;
	Item *ant;
	
	/**
	 * Se busca el cliente en la lista de clientes,
	 * 	de su lista interna se obitenen las salas,
	 * 	se busca en esas salas para eliminar al cliente
	 * 	de las listas internas. 
	 */
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
		eliminar(cliente->listaInterna, ant);
	}

	return "Se ha desuscrito de todas sus salas con exito.\n";
}

/**
 * Funcion encargada de crear una sala.
 * 
 * @param Nombre de la sala la cual sera insertada en la lista
 * 	de salas activas.
 * @return String con la respuesta del servidor al cliente.
 * 
 */
char *crearSala(char *nombreSala){
	int i;
	Item *item;
	
	i = insertar(salas, nombreSala, 0);

	/**
	 * Al insertar en la lista se inicializa la lista interna.
	 */
	if (i == 1) {
		pthread_mutex_lock(&(salas->bodyguard));
		item = buscar(salas, nombreSala);
		pthread_mutex_unlock(&(salas->bodyguard));
		
		item->listaInterna = calloc(1, sizeof(Lista));
		
		if(item->listaInterna == NULL){
			fatalerror("No se puede alocar memoria para una lista interna.\n");
			return "No se pudo crear sala por problemas internos en el servidor.\n";
		}
		
		if (pthread_mutex_init(&(item->listaInterna->bodyguard), NULL) != 0) {
			fatalerror("No se puede inicializar el mutex de una lista.\n");
			return "No se pudo crear sala por problemas internos en el servidor.\n";
		} 
		
	}else {
		return "Esta sala ya existe.\n";
	}
	return "Sala creada con exito.\n";
}

/**
 * Funcion encargada de eliminar una sala.
 * 
 * @param Nombre de la sala la cual sera eliminada en la lista
 * 	de salas activas.
 * @return String con la respuesta del servidor al cliente.
 * 
 */
char *eliminarSala(char *sala){
	Item *item;
	
	if(strcmp(sala, nombreSala) == 0){
		return "No puede eliminar la sala predeterminada del chat\n";
	}
	
	/* Se busca la sala a ser eliminada */
	pthread_mutex_lock(&(salas->bodyguard));
	item = buscar(salas, sala);
	pthread_mutex_unlock(&(salas->bodyguard));
	if (item == NULL) {
		return "No se pudo eliminar la sala por problemas internos en el servidor.\n";
	} else {
		/**
		 * Se limpia la lista interna de la sala y se 
		 *	eliminan de las salas internas de los usuarios
		 * 	los cuales estaban suscritos a la sala.
		 */
		
		Item *cliente;
		Item *aux;
		Item *sentenciado;
		Item *ant;
		
		aux = item->listaInterna->primero;

		while (aux != NULL) {
			
			pthread_mutex_lock(&(clientes->bodyguard));
			cliente = buscar(clientes, aux->name);
			pthread_mutex_unlock(&(clientes->bodyguard));
		
			pthread_mutex_lock(&(cliente->listaInterna->bodyguard));
			sentenciado = buscar(cliente->listaInterna, sala);
			pthread_mutex_unlock(&(cliente->listaInterna->bodyguard));
		
			eliminar(cliente->listaInterna, sentenciado);
			ant = aux;
			aux = aux->ApSig;
			eliminar(item->listaInterna, ant);
		}
		eliminar(salas, item);
		
		return "Sala eliminada con exito\n";
		
	}
}

/**
 * Funcion encargada de la eliminacion de un cliente.
 * 
 * @param Nombre del cliente.
 * 
 */
void abandonarCliente(char *nombreCliente){	
 	Item *item5;
	
	/* Se desuscribe al cliente de todas las salas que pertene */
	desuscribirSala(nombreCliente);
	
	pthread_mutex_lock(&(clientes->bodyguard));
	item5 = buscar(clientes, nombreCliente);
	pthread_mutex_unlock(&(clientes->bodyguard));
	
	/* Se elimina el cliente de los usuarios activos. */
	eliminar(clientes,item5);
	return;
 }

/**
 * 
 * Funcion encargada de la atencion al cliente. 
 * 
 * @param Escructura que contiene:
 * * El id del hilo.
 * * El nombre del cliente.
 * * El socket del cliente.
 * 
 */
void *atenderCliente(ParametrosHilos *recibe){
	
	ParametrosHilos *recibe2 = calloc(1, sizeof(ParametrosHilos));
	
	if (recibe2 == NULL){
		free(recibe2);
		fatalerror("No se ha podido reservar memoria mediante calloc.\n");
	}
	
	recibe2->newsockfd = recibe->newsockfd;
	recibe2->id = recibe->id;
	
	inicializarCliente(recibe2);
	
	char *comando = calloc(BUFFERTAM, sizeof(char));
	char *respuesta = calloc(BUFFERTAM, sizeof(char));
	char *respuestaBonita = calloc(BUFFERTAM,sizeof(char));
	Item *item5;
	
	while (abortar == 0) {
		/* Se lee lo enviado por el cliente */
		if (read(recibe2->newsockfd, comando, BUFFERTAM) < 0) {
			fatalerror("can't read the socket");
		}
		if (abortar == 1){
			free(comando);
			free(respuesta);
			pthread_exit(&hilos[recibe2->id]);
		}
		
		/**
		 * Se verifica cual comando es enviado para cumplir
		 * con la funcion especifica de cada uno
		 */
		switch (comando[0]) {
			case 's':
				if(comando[1] == 'a'){
					strcpy(respuestaBonita, "Lista de salas del servidor:\n");
					strcat(respuestaBonita, listar(salas));
					respuesta = respuestaBonita;
				} else {
					respuesta = suscribirSala(comando+4, recibe2->nombreCliente, recibe2->newsockfd);
				}
				break;
			case 'u': 
				strcpy(respuestaBonita, "Lista de clientes activos:\n");
				strcat(respuestaBonita, listar(clientes));
				respuesta = respuestaBonita;
				break;
			case 'm': 
				respuesta = enviarMensaje(comando + 4, recibe2->nombreCliente);
				break;
			case 'd': 
				respuesta = desuscribirSala(recibe2->nombreCliente);
				break;
			case 'c':
				respuesta = crearSala(comando + 4);
				break;
			case 'e':
				respuesta = eliminarSala(comando + 4);
				break;
			case 'f': 
				strcpy(respuestaBonita, comando);
				respuesta = respuestaBonita;
				break;
			default:
				printf("Error de protocolo con uno de los clientes\n");
				free(comando);
				free(respuesta);
				pthread_exit(&hilos[recibe2->id]);
		}
		
		/* Se envia una respuesta al cliente. */
		if (write(recibe2->newsockfd, respuesta, BUFFERTAM) < 0){
				fatalerror("can't write to socket");
		}
		
		if (comando[0] == 'f') {
			abandonarCliente(recibe2->nombreCliente);
			break;
		}
	}
	
	free(comando);
	free(respuesta);
	pthread_exit(&hilos[recibe2->id]);
}

/**
 * Funcion principal del schat, encargada de la
 *	conexion del cliente al servidor. 
 * 
 * @param Cantidad de argumentos enviados.
 * @param Argumentos enviados.
 * 
 */
int main(int argc, char *argv []) {
	/* Variables para la conexion y comunicacion */
	int newsockfd;
	struct sockaddr_in clientaddr, serveraddr;
	int clientaddrlength;
	int i = 0;
	ParametrosHilos envio;
	char key;
	nombreSala = calloc(BUFFERTAM+1, sizeof(char));
	
	
	if (nombreSala == NULL) {
		fatalerror("Error reservando memoria mediante calloc");
	} else {
		nombreSala = "actual";
	}
	
	
	/* Se inicializa la lista de los clientes */
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
	
	signal(SIGINT, abortarSeguro);
	
	/* Abre el socket TCP. */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		fatalerror("No abre el socket D:");
	}
	/* Exprerando por el socket. */
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
		/* Esperando por la conecxion. */
		clientaddrlength = sizeof(clientaddr);
		newsockfd = accept(sockfd, 
						(struct sockaddr *) &clientaddr,
						&clientaddrlength);
		
		if (newsockfd < 0){
			fatalerror("accept failure");
		}
		
		envio.id = hilosEnArreglo++;
		envio.newsockfd = newsockfd;
		if ((pthread_create(&hilos[i], NULL, (void *)atenderCliente, (void *)&envio)) != 0){
			fatalerror("Error catastrofico creando hilo :OOO");
		}
		
		
	}
		
}
