#il compilatore:
CC=gcc

# CFLAGS will be the options passed to the compiler.
CFLAGS=-c -g -Wall -pthread -pg

#oggetti da creare:
OBJECTS = Utils.o OPEN.o CLOSE.o READ.o WRITE.o Cache.o Heartbeating.o Test.o

#Creera' tutti questi binari:
all:prog testopen testread testheartbeating testinvalidazione testfillfs testfilebinario testalltest testopenerrors testreaderrors

prog:libmydfs.a
	gcc -o TestWrite -g -pg -pthread testWrite.c libmydfs.a

%.o: %.c
	$(CC) $(CFLAGS) $< 

libmydfs.a:$(OBJECTS)
	ar rvs libmydfs.a $(OBJECTS)

clean:
	rm -rf *.o

testread:
	gcc -o TestRead -g -pg -pthread testRead.c libmydfs.a

testopen:
	gcc -o TestOpen -g -pg -pthread testOpen.c libmydfs.a

testheartbeating:
	gcc -o TestHeartBeating -g -pg -pthread testHeartBeating.c libmydfs.a

testinvalidazione:
	gcc -o TestInvalidazione -g -pg -pthread testInvalidazione.c libmydfs.a

testfillfs:
	gcc -o TestFillFS -g -pg -pthread testFillFS.c libmydfs.a
	
testfilebinario:
	gcc -o TestBinario -g -pg -pthread testBinario.c libmydfs.a

testopenerrors:
	gcc -o TestOpenErrors -g -pg -pthread testOpenErrors.c libmydfs.a

testreaderrors:
	gcc -o TestReadErrors -g -pg -pthread testOpenErrors.c libmydfs.a


testalltest:
	gcc -o TestAll -g -pthread testAllTests.c libmydfs.a
