#Make file del lato server

CC=gcc
# CFLAGS will be the options passed to the compiler.
CFLAGS=-c -Wall -g
OBJECTS = CommandsHandler.o Utils.o Error.o server.o OPE.o BYE.o StruttureDati.o

all:prog

prog:$(OBJECTS)
	$(CC) $(OBJECTS) -o Server
	
%.o: %.c
	$(CC) $(CFLAGS) $< 

clean:
	rm -rf *.o
