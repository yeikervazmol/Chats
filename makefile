CC = gcc
CFLAGS = -Wall

all : tcpechoclient schat

errors.o : errors.c errors.h
	$(CC) $(CFLAGS) -c errors.c

#tcpechoserver : tcpechoserver.c errors.o
#	$(CC) $(CFLAGS) -o tcpechoserver tcpechoserver.c errors.o $(LIBS)

tcpechoclient : tcpechoclient.c errors.o
	$(CC) $(CFLAGS) -pthread -o tcpechoclient tcpechoclient.c errors.o $(LIBS)
	
schat : schat.c errors.o
	$(CC) $(CFLAGS) -pthread -o schat schat.c errors.o $(LIBS)
	
lista : 
	gcc Lista.c -Wall -pthread -o Lista

clean:
	rm *.o schat tcpechoclient