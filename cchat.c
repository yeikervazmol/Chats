/**
* @author Karen Troiano		09-10855
* @author Yeiker Vazquez	09-10855
* @grupo  
*
* Archivo: cchat.c
*
* Descripcion: Contiene el programa principal del
* cliente del chat.
*/

#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <signal.h>
#include "errors.h"
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFFERTAM 1024

/**
 * Variables globales que seran compartidas por los hilos.
 */

/* Hilos utilizados para escritura y lectura al servidor */
pthread_t hiloEC, hiloRM; 
/* Filedescriptor del archivo que podrian enviar */
FILE *fp = NULL;
/* Variable para saber si se lee de archivo o entrada estandar. */
int archivoActivo = 0;
/* 
 * Variable para saber si el cliente ha abortado 
 *	abrutamente el programa 
 */
int abortar = 0;
/* Variable del socket del cliente */
int sockfd;
/* 
 * Variable para saber si el cliente ha servidor 
 *	abrutamente el programa 
 */
int servidorActivo = 0;
char *nombre;
/**
 * Fin de las variables globales.
 */

/**
 * 
 * Funcion encargada de abortar ejecucion si el
 * 	cliente envia una interrupcion al programa
 * 	durante su ejecucion.
 * 
 */
void abortarSeguro(){
	
	abortar = 1;
	if (servidorActivo == 1) {
		if (write(sockfd, "fue", BUFFERTAM) < 0) {
			fatalerror("can't write to socket");
		}
		printf("\nHasta luego (de manera abrupta).\n");
	} else {
		printf("\nEl servidor ha dejado de funcionar.\n");
	}
	free(nombre);
	close(sockfd);
	exit(1);
}

/**
 * Funcion encargada de filtrar malos comandos hacia el servidor.
 * 
 * @param El comando a ser verificado.
 * @return Devuelve 1 si el comando esta correcto, 0 en caso
 *	 contrario.
 * 
 */
int filtrar(char *paquete) {
	char *comando = calloc(5,sizeof(char));
	if (comando == NULL) {
		fatalerror("Error reservando memoria mediante calloc");
	}
	/**
	 * Tomamos solo las 3 o 4 primeras palabras del
	 * 	comando para saber si es correcto.
	 */
	if (strlen(paquete) == 3) {
		strncpy(comando, paquete, 3);
		comando[4] = '\0';
	} else {
		strncpy(comando, paquete, 4);
		comando[5] = '\0';
	}
	/**
	 * Comparamos lo recibido con lo permitido del
	 * 	chat para ejecutar comandos.
	 */
	if( (strcmp(comando, "sal") == 0)
		||	(strcmp(comando, "usu") == 0)
		|| 	(strcmp(comando, "men ") == 0)
		|| 	(strcmp(comando, "sus ") == 0)
		|| 	(strcmp(comando, "des") == 0)
		|| 	(strcmp(comando, "cre ") == 0)
		|| 	(strcmp(comando, "eli ") == 0)
		|| 	(strcmp(comando, "fue") == 0) ) {
		free(comando);
		return 1;
	} else {
		free(comando);
		return 0;
	}
}

/**
 * Funcion encargada de recibir un comando por archivo o terminal,
 * 	filtrar el mensaje para verificar su correctitud y envia al
 * 	servidor dicho mensaje para su debida respuesta. 
 * 
 * @param El numero del socket del cliente. 
 * 
 */
void *enviarComando(int *sockfd) {
	
	char *mensaje = calloc(BUFFERTAM+1, sizeof(char));
	if (mensaje== NULL) {
		fatalerror("Error reservando memoria mediante calloc");
	}
	int j = BUFFERTAM;
	int i = 0;
	int longMensaje;
	
	/* Se escribe hasta que el programa aborte abrutamente */
	while (abortar == 0){
		
		/* Si existe el archivo leemos linea por linea */
		if (archivoActivo == 1) {
			if (fgets(mensaje, j, fp) == NULL) {
				archivoActivo = 0;
				/* Se cierra el archivo pues termina su lectura */
				if (fclose(fp)!= 0) {
					printf( "Problemas al cerrar el fichero\n" );
				}
			}
		} 
		/**
		 * Si el archivo no existe o se leyo completamente, se 
		 * espera por entrada estandar para esperar por los nuevos 
		 * comandos 
		 */
		if (archivoActivo == 0) {
			getline(&mensaje, &j, stdin);
			if (abortar == 1){
				free(mensaje);
				pthread_exit(&hiloEC);
			}
		}
		
		longMensaje = strlen(mensaje) - 1;
		
		if (mensaje[longMensaje] == '\n') {
			mensaje[longMensaje] = '\0';
		}
		
		/* Se verifica la correctitud del mensaje enviado */
		if(filtrar(mensaje)) {
			
			/** 
			 * Se escribe en el socket el mensaje a ser enviado
			 * al servidor.
			 */
			if (write(*sockfd, mensaje, BUFFERTAM) < 0) {
				servidorActivo = 0;
				free(mensaje);
				abortarSeguro();
			}
		
			/* Si el comando es fue, se intenta abortar ejecucion */
			if (strcmp(mensaje, "fue") == 0) {
				if (archivoActivo == 1) {
					if (fclose(fp)!= 0) {
						printf( "Problemas al cerrar el fichero\n" );
					}
				}
				archivoActivo = 0;
				abortar = 1;
				break;
			}
			
		} else {
			printf("Sintaxis incorrecta, pruebe con:\n "
			"sal: Este comando hace que el usuario pueda ver en su pantalla " 
			"una lista de las salas de chat que el servidor posee.\n"
			"usu: Este comando hace que el usuario pueda ver en su pantalla "
			"una lista actualizada de todos los usuarios que están suscritos "
			"en el servidor, incluyéndolo a el mismo\n"
			"men <mensaje>: Este comando envía el mensaje a todos los usuarios"
			" que están conectados al mismo servidor en la sala de chat a la "
			"que está suscrito el usuario.\n"
			"sus <sala>: El usuario se suscribe a la sala de chat sala.\n"
			"des Este comando de-suscribe al usuario de la sala o salas a las "
			"que este suscrito\n"
			"cre <sala>: El usuario crea la sala en el servidor.\n"
			"eli <sala>: El usuario elimina la sala del servidor.\n"
			"fue: Este comando permite terminar l\n");
		}
	}
		
	if (archivoActivo == 1) {
		if (fclose(fp)!= 0) {
			printf( "Problemas al cerrar el fichero\n" );
		}
	}
	
	free(mensaje);
	pthread_exit(&hiloEC);
}

/**
 * Funcion encargada de recibir un mensaje del servidor e 
 * 	imprimirlo por salida estandar.
 * 
 * @param El numero del socket del cliente. 
 * 
 */
void *recibirMensaje(int *sockfd){
	char *recibido = calloc(BUFFERTAM, sizeof(char));
	if (recibido == NULL) {
		fatalerror("Error reservando memoria mediante calloc");
	}
	
	/* Hasta que no se corte la ejecucion abrutamente hacer */
	while(abortar == 0){
		
		/* Se lee esperando por el servidor */
		if (read(*sockfd, recibido, BUFFERTAM) < 0) {
			servidorActivo = 0;
			free(recibido);
			abortarSeguro();
		}
		
		if (abortar == 1){
			free(recibido);
			pthread_exit(&hiloRM);
		}
		
		if (strcmp(recibido, "fue") == 0) {
			
			/* Caso en el que el servidor aborta. */
			if(abortar == 0){
				servidorActivo = 0;
				abortar = 1;
				abortarSeguro();
				break;
			} else {
				/* Caso en el que el cliente aborta. */
				break;
			}
		}
		/* Imprimimos el mensaje recibido del servidor */
		printf("%s", recibido);
	}
	
	free(recibido);
	pthread_exit(&hiloRM);
}

/**
 * 
 * Funcion encargada de crear dos hilos:
 * * &hiloEC: Envia comandos al servidor.
 * * &hiloRM Recibe mensajes del servidor. 
 * 
 * @param El numero del socket del cliente. 
 * 
 */
void iniciarConcurrencia(int sockfd) {
	pthread_create(&hiloEC, NULL, (void *)enviarComando, &sockfd);
	pthread_create(&hiloRM, NULL, (void *)recibirMensaje, &sockfd);
	pthread_join(hiloEC, NULL);
	pthread_join(hiloRM, NULL);
	printf("Hasta luego.\n");
}

/**
 * Funcion principal del cchat, encargada de la
 *	conexion del cliente al servidor. 
 * 
 * @param Cantidad de argumentos enviados.
 * @param Argumentos enviados.
 * 
 */
int main(int argc, char *argv[]) {
	
	/* Variable que verifica si el nombre del usuario no existe */
	int identidadValida = 0;
	struct sockaddr_in serveraddr;
	/**
	 * Variables que contiene el servidor y puerto
	 * * Por default el servidor es 127.0.0.1
	 * * Por default puerto es 25504 
	 */
	char *server;
	server = "127.0.0.1";
	int puerto = 25504;
	/* Variables que contienen los nombres de usuario y archivo */
	nombre = calloc(BUFFERTAM+1, sizeof(char));
	char *archivo = NULL;
	char key;
	int j = BUFFERTAM;
	int lenNombre;
	struct hostent *host;
	
	if (nombre == NULL) {
		fatalerror("Error reservando memoria mediante calloc");
	}
	
	/* Recordar el nombre del programa para enviar errores. */
	programname = argv[0];

	while ((key = getopt(argc, argv, "h:p:n:a:")) != -1) {
		switch (key) {
			case 'p':
				puerto = atoi(optarg);
				break;
			case 'n':
				strcpy(nombre, optarg);
				break;
			case 'h':
				server = optarg;
				break;
			case 'a':
				archivo = optarg;
				break;	
			default:
				printf("Forma incorrecta de invocacion del programa," 
					   "mejor intete: cchat [-h <host>] [-p <puerto>]"
					   "[-n <nombre>][-a <archivo>]\n");
				exit(EXIT_FAILURE);
				break;
			}
	}

		
	signal(SIGINT, abortarSeguro);
	
	if ((host = gethostbyname(server)) == NULL) {
			fatalerror("No se pudo encontrar la direccion IP.\n");
	}
	
	/* Get the address of the server. */
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr = *((struct in_addr *)host->h_addr);
	serveraddr.sin_port = htons(puerto);

	/* Abriendo el socket. */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		fatalerror("can't open socket");

	/* Conectarnos al servidor */
	if (connect(sockfd, (struct sockaddr *) &serveraddr,
				sizeof(serveraddr)) < 0)
		fatalerror("can't connect to server");
	
	servidorActivo = 1;
	
	/* Enviamos el nombre del cliente al socket */
	while( strcmp(nombre, "") == 0 ){
		printf("Introduzca su alias en el chat: ");
		getline(&nombre, &j, stdin);
		lenNombre = strlen(nombre) - 1;
		nombre[lenNombre] ='\0';
	}
	
	if (write(sockfd, nombre, BUFFERTAM) < 0){
		fatalerror("can't write to socket");
	}
	
	if (read(sockfd, &identidadValida, sizeof(int)) < 0) {
		fatalerror("can't read from socket");
	}	

	/* identidadValida vale 1 si el nombre de usuario de valido
	 y 0 cuando el nombre de usuario ya esta siendo utilizado.*/
	if (identidadValida == 0) {
		while(!identidadValida){
			printf("%s es un usuario existente.\n"
				"Por favor, seleccione otro nombre de usuario: ", nombre);
			
			getline(&nombre, &j, stdin);
			lenNombre = strlen(nombre) - 1;
			nombre[lenNombre] ='\0';
			if (write(sockfd, nombre, BUFFERTAM) < 0){
				fatalerror("can't write to socket");
			}
			
			if (read(sockfd, &identidadValida, sizeof(int)) < 0) {
				fatalerror("can't read from socket");
			}
		}
	} 
	
	printf("%s, bienvenido al chat.\n", nombre);
	
  /*Lectura de archivo*/
	if (archivo != NULL){
		archivoActivo = 1;
		/*Se abre el archivo.*/ 
		if ( (fp = fopen(archivo, "r")) == NULL) {
			printf ( " Error en la apertura del archivo. Es posible que el fichero no"
						" exista\n");
		}
		
	}
	
  /* Copy input to the server. */
	iniciarConcurrencia(sockfd);
	close(sockfd);
	exit(EXIT_SUCCESS);
}

