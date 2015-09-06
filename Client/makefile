CC=gcc
# CFLAGS will be the options passed to the compiler.
CFLAGS=-c -g -Wall -pthread
OBJECTS = Utils.o OPEN.o CLOSE.o READ.o WRITE.o Cache.o Heartbeating.o Test.o

all:prog

prog:libmydfs.a
	gcc -o TestWrite -g -pthread testWrite.c libmydfs.a

%.o: %.c
	$(CC) $(CFLAGS) $< 

libmydfs.a:$(OBJECTS)
	ar rvs libmydfs.a $(OBJECTS)

clean:
	rm -rf *.o

testread:
	gcc -o TestRead -g -pthread testread.c libmydfs.a

testopen:
	gcc -o TestOpen -g -pthread testopen.c libmydfs.a

testheartbeating:
	gcc -o TestHeartBeating -g -pthread testHeartBeating.c libmydfs.a
