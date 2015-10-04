#il compilatore:
CC=gcc

# CFLAGS will be the options passed to the compiler.
CFLAGS=-c -g -Wall -pthread

#oggetti da creare:
OBJECTS = Utils.o OPEN.o CLOSE.o READ.o WRITE.o Cache.o Heartbeating.o Test.o

#Creera' tutti questi binari:
all:prog testopen testread testheartbeating testinvalidazione teststresstest testfillfs testfilebinario

prog:libmydfs.a
	gcc -o TestWrite -g -pthread testWrite.c libmydfs.a

%.o: %.c
	$(CC) $(CFLAGS) $< 

libmydfs.a:$(OBJECTS)
	ar rvs libmydfs.a $(OBJECTS)

clean:
	rm -rf *.o

testread:
	gcc -o TestRead -g -pthread testRead.c libmydfs.a

testopen:
	gcc -o TestOpen -g -pthread testOpen.c libmydfs.a

testheartbeating:
	gcc -o TestHeartBeating -g -pthread testHeartBeating.c libmydfs.a

testinvalidazione:
	gcc -o TestInvalidazione -g -pthread testInvalidazione.c libmydfs.a

teststresstest:
	gcc -o TestStressTest -g -pthread testStressTest.c libmydfs.a

testfillfs:
	gcc -o TestFillFS -g -pthread testFillFS.c libmydfs.a
testfilebinario:
	gcc -o TestBinario -g -pthread testBinario.c libmydfs.a
