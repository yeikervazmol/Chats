CC = gcc
CFLAGS = -g

all : cchat schat

errors.o : errors.c errors.h
	$(CC) $(CFLAGS) -c errors.c

lista : Lista.c Lista.h
	$(CC) Lista.c $(CFLAGS) -pthread -o Lista
	
cchat : cchat.c errors.o Lista.o
	$(CC) $(CFLAGS) -pthread -o cchat cchat.c errors.o Lista.o

schat : schat.c errors.o Lista.o
	$(CC) $(CFLAGS) -pthread -o schat schat.c errors.o Lista.o
	
clean:
	rm *.o schat cchat
