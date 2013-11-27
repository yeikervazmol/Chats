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

main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_in serveraddr;
  char *server;
  server = "127.0.0.1";
  int puerto = 25504;
  char *nombre = "anonimo";
  char *archivo;
  char key;
  FILE *fp = NULL;
  char ch;
  
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
				printf("Forma incorrecta de invocacion del programa, mejor intete: cchat [-h <host>] [-p <puerto>] [-n <nombre>][-a <archivo>]\n");
				exit(EXIT_FAILURE);
				break;
			}
	}
 
	/*Lectura de archivo*/
	/*/*Se abre el archivo. 
	if ( (fp = fopen(archivo, "r")) == NULL) {
		printf ( " Error en la apertura del archivo. Es posible que el fichero no"
					" exista \n ");
	};
	
	while((ch = fgetc(fp)) != EOF) {
		printf("%c",ch);
	}
	
	/*Se cierra el archivo.
	if (fclose(fp)!= 0)
	{
		printf( "Problemas al cerrar el fichero\n" );
	}
	
	*/
  /* Get the address of the server. */
  bzero(&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = inet_addr(server);
  serveraddr.sin_port = htons(puerto);

  /* Open a socket. */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    fatalerror("can't open socket");

  /* Connect toserveraddr the server. */
  if (connect(sockfd, (struct sockaddr *) &serveraddr,
              sizeof(serveraddr)) < 0)
    fatalerror("can't connect to server");

  /* Copy input to the server. */
  copy(sockfd);
  close(sockfd);

  exit(EXIT_SUCCESS);
}

