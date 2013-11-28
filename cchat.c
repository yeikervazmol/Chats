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
#include "errors.h"
#include <pthread.h>

#define BUFFERTAM 1024

void *getAndWrite(int *sockfd) {
	char c;
	char mensaje[140];
	int i;
	
	while (1){
		for (i=0; i<140; i++){
			mensaje[i] = '\0';
		}
		i = 0;
		while ((c = getchar()) != '\n'){
			mensaje[i++] = c;
		}
		
		if (write(*sockfd, mensaje, 140) < 0){
			fatalerror("can't write to socket");
		}
	}
}

void *readAndPrint(int *sockfd){
	int *sfd = sockfd;
	char recibido[140];
	int i;
	while(1){
		for (i=0; i<140; i++){
			recibido[i] = '\0';
		}
		if (read(*sfd, recibido, 140) < 0) {
			fatalerror("can't read from socket");
		}
		printf("Recibido desde el servidor: %s\n", recibido);
	}
}

void copy(int sockfd) {
	pthread_t hiloW, hiloR;
	pthread_create(&hiloW, NULL, (void *)getAndWrite, &sockfd);
	pthread_create(&hiloR, NULL, (void *)readAndPrint, &sockfd);
	pthread_join(hiloW, NULL);
	pthread_join(hiloR, NULL);
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
			printf ( 	" Error en la apertura del archivo. Es posible que el fichero no"
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
