#
# @author Karen Troiano		09-10855
# @author Yeiker Vazquez	09-10855
# @grupo  
#
# Archivo -> makefile
#
# Descripcion ->  Contiene el archivo que compila
#	todos los elementos necesarios para el
#	programa.
#
#

CC = gcc
CFLAGS = -g

all : cchat schat

errors.o : errors.c errors.h
	$(CC) -c errors.c

lista : Lista.c Lista.h
	$(CC) Lista.c -pthread -o Lista
	
cchat : cchat.c errors.o Lista.o
	$(CC) -pthread -o cchat cchat.c errors.o Lista.o

schat : schat.c errors.o Lista.o
	$(CC) -pthread -o schat schat.c errors.o Lista.o

debug : cchatD schatD
	

cchatD : cchat.c errors.o Lista.o
	$(CC) $(CFLAGS) -pthread -o cchat cchat.c errors.o Lista.o

schatD : schat.c errors.o Lista.o
	$(CC) $(CFLAGS) -pthread -o schat schat.c errors.o Lista.o
	
clean:
	rm *.o schat cchat
