/*
* @author Karen Troiano && Yeiker Vazquez
* @carnet 09-10855 && 09-10855
* @grupo  09
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
#include "errors.h"
#include <pthread.h>

#define BUFFERTAM 1024
pthread_mutex_t _mutex;

int filtrar(char *paquete) {
	char *comando = calloc(5,sizeof(char));
	if (strlen(paquete) == 3) {
		strncpy(comando, paquete, 3);
		comando[4] = '\0';
	} else {
		strncpy(comando, paquete, 4);
		comando[5] = '\0';
	}
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

void *recibeComando(int *sockfd) {
	
	char *mensaje = calloc(BUFFERTAM+1, sizeof(char));
	char *recibido = calloc(BUFFERTAM, sizeof(char));
	int j = BUFFERTAM;
	int i = 0;
	int longMensaje;
	
	while (1){
		
		getline(&mensaje, &j, stdin);
		longMensaje = strlen(mensaje) - 1;
		mensaje[longMensaje] = '\0';
		
		if(filtrar(mensaje)) {
		
			if (write(*sockfd, mensaje, BUFFERTAM) < 0) {
				fatalerror("can't write to socket");
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
}

void *recibeMensaje(int *sockfd){
	char *recibido = calloc(BUFFERTAM, sizeof(char));
	int i;
	while(1){
		
		if (read(*sockfd, recibido, BUFFERTAM) < 0) {
			fatalerror("can't read from socket");
		}
		
		
		printf("%s", recibido);
	}
}

void copy(int sockfd) {
	pthread_t hiloRC, hiloRM;
	pthread_create(&hiloRC, NULL, (void *)recibeComando, &sockfd);
	pthread_create(&hiloRM, NULL, (void *)recibeMensaje, &sockfd);
	pthread_join(hiloRC, NULL);
	pthread_join(hiloRM, NULL);
}

int main(int argc, char *argv[]) {
	int sockfd;
	int identidadValida = 0;
	struct sockaddr_in serveraddr;
	char *server;
	server = "127.0.0.1";
	int puerto = 25504;
	char *nombre = calloc(BUFFERTAM+1, sizeof(char));
	char *archivo = NULL;
	char key;
	FILE *fp = NULL;
	char line[BUFFERTAM];
	int j = BUFFERTAM;
	int lenNombre;
  
  /* Remember the program name for error messages. */
  programname = argv[0];

	while ((key = getopt(argc, argv, "h:p:n:a:")) != -1) {
		switch (key) {
			case 'p':
				puerto = atoi(optarg);
				break;
			case 'n':
				nombre = optarg;
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

	/* Get the address of the server. */
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(server);
	serveraddr.sin_port = htons(puerto);

	/* Abriendo el socket. */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		fatalerror("can't open socket");

	/* Conectarnos al servidor */
	if (connect(sockfd, (struct sockaddr *) &serveraddr,
				sizeof(serveraddr)) < 0)
		fatalerror("can't connect to server");
	
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
		
		/*Se abre el archivo.*/ 
		if ( (fp = fopen(archivo, "r")) == NULL) {
			printf ( " Error en la apertura del archivo. Es posible que el fichero no"
						" exista\n");
		} else {
			while (fgets(line, sizeof(line), fp)) {
				printf("%s", line); 
			}
			
			if (fclose(fp)!= 0) {
				printf( "Problemas al cerrar el fichero\n" );
			}
		}
		
	}
  /* Copy input to the server. */
  copy(sockfd);
  close(sockfd);

  exit(EXIT_SUCCESS);
}

