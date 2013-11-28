CC = gcc
#CFLAGS = -Wall

all : cchat schat

errors.o : errors.c errors.h
	$(CC) $(CFLAGS) -c errors.c

cchat : cchat.c errors.o
	$(CC) $(CFLAGS) -pthread -o cchat cchat.c errors.o
	
schat : schat.c errors.o
	$(CC) $(CFLAGS) -pthread -o schat schat.c errors.o
	
lista : 
	$(CC) Lista.c $(CFLAGS) -pthread -o Lista

clean:
	rm *.o schat cchat
