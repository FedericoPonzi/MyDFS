#Make file del lato server

CC=gcc
# CFLAGS will be the options passed to the compiler.
CFLAGS=-c -Wall -g -pthread
OBJECTS = CommandsHandler.o Utils.o server.o OPE.o StruttureDati.o CLOSE.o Config.o Heartbeating.o READ.o

all:prog

prog:$(OBJECTS)
	$(CC) -pthread $(OBJECTS) -o Server
	
%.o: %.c
	$(CC) $(CFLAGS) $< 

clean:
	rm -rf *.o
