/************************************************************
tcpechoclient.c

This is a client for the tcp echo server.  It sends anything
read from standard input to the server, reads the responses,
and sends them to standard output.

Copyright (C) 1995 by Fred Sullivan      All Rights Reserved
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include "errors.h"
#include <pthread.h>

#define PORT 25503

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

  /* Remember the program name for error messages. */
  programname = argv[0];

  /* Who's the server? */
  if (argc == 2)
    server = argv[1];
  else
    server = "127.0.0.1";

  /* Get the address of the server. */
  bzero(&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = inet_addr(server);
  serveraddr.sin_port = htons(PORT);

  /* Open a socket. */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    fatalerror("can't open socket");

  /* Connect to the server. */
  if (connect(sockfd, (struct sockaddr *) &serveraddr,
              sizeof(serveraddr)) < 0)
    fatalerror("can't connect to server");

  /* Copy input to the server. */
  copy(sockfd);
  close(sockfd);

  exit(EXIT_SUCCESS);
}
