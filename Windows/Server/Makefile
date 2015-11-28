#Make file del lato server

CC=gcc
# CFLAGS will be the options passed to the compiler.
CFLAGS=-c -Wall -g -ggdb
OBJECTS = CommandsHandler.o Utils.o OPE.o StruttureDati.o CLOSE.o Config.o Heartbeating.o READ.o

all:prog multiprocess

prog:$(OBJECTS)
	$(CC) $(OBJECTS) -g -ggdb -o Server server.c -D_WIN32_WINNT=0x0501 -lws2_32

multiprocess:$(OBJECTS)
	$(CC) $(OBJECTS) -g -ggdb -o ProcessServer ProcessServer.c -D_WIN32_WINNT=0x0501 -lws2_32

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o Server.exe ProcessServer.exe
