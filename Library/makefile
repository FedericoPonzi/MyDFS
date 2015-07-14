CC=gcc
# CFLAGS will be the options passed to the compiler.
CFLAGS=-c 
OBJECTS = Utils.o FileHandler.o

all:prog

prog:$(OBJECTS)
	ar rvs libmydfs.a $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) $< 

clean:
	rm -rf *.o
