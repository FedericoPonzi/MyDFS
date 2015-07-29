CC=gcc
# CFLAGS will be the options passed to the compiler.
CFLAGS=-c -g -Wall 
OBJECTS = Utils.o FileHandler.o OPEN.o CLOSE.o READ.o WRITE.o

all:prog

prog:libmydfs.a
	gcc -o Client -g client.c libmydfs.a

%.o: %.c
	$(CC) $(CFLAGS) $< 

libmydfs.a:$(OBJECTS)
	ar rvs libmydfs.a $(OBJECTS)

clean:
	rm -rf *.o
