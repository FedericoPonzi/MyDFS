#il compilatore:
CC=gcc

# CFLAGS will be the options passed to the compiler.
CFLAGS=-c -g -Wall 

#oggetti da creare:
OBJECTS = Utils.o OPEN.o CLOSE.o READ.o WRITE.o Cache.o Heartbeating.o Test.o

#Creera' tutti questi binari:
all: testHeartbeating
#testopen 
#testread testheartbeating testinvalidazione 
#teststresstest

prog:libmydfs.a
	gcc -o TestWrite -g testWrite.c libmydfs.a -lWs2_32

%.o: %.c
	$(CC) $(CFLAGS) $< -lWs2_32

libmydfs.a:$(OBJECTS)
	ar rvs libmydfs.a $(OBJECTS)

clean:
	rm -rf *.o

testread:
	gcc -o TestRead -g testRead.c libmydfs.a -lWs2_32

testheartbeating:
	gcc -o TestHeartBeating -g testHeartBeating.c libmydfs.a -lWs2_32 

testinvalidazione:
	gcc -o TestInvalidazione -g testInvalidazione.c libmydfs.a -lWs2_32

testopen:
	gcc -o TestOpen -g testOpen.c libmydfs.a -lWs2_32

#teststresstest:
#	gcc -o TestStressTest -g testStressTest.c libmydfs.a -lWs2_32
