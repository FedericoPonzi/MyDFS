#il compilatore:
CC=gcc

# CFLAGS will be the options passed to the compiler.
CFLAGS=-c -g -Wall 

LIBS= libmydfs.a -lWs2_32 

#oggetti da creare:
OBJECTS = Utils.o OPEN.o CLOSE.o READ.o WRITE.o Cache.o Heartbeating.o Test.o

#Creera' tutti questi binari: testread   testfillfs testfilebinario testalltest testopenerrors testreaderrors
all::prog topen  testinvalidazione testheartbeating testread  testfilfs testfilebinario testopenerrors
 
prog:libmydfs.a
	gcc -o TestWrite -g testWrite.c libmydfs.a -lWs2_32 

%.o: %.c
	$(CC) $(CFLAGS) $<

libmydfs.a:$(OBJECTS)
	ar rvs libmydfs.a $(OBJECTS)

clean:
	rm -rf *.o TestRead TestOpen TestHeartBeating TestInvalidazione TestFillFs TestBinario TestOpenErrors TestWrite TestFillFS 

testread:
	gcc -o TestRead -g testRead.c $(LIBS)

testheartbeating:
	gcc -o TestHeartBeating -g testHeartBeating.c $(LIBS)

testinvalidazione:
	gcc -o TestInvalidazione -g testInvalidazione.c $(LIBS)

topen:
	gcc -o TestOpen -g testOpen.c $(LIBS)

testfilfs:
	gcc -o TestFillFS -g testFillFS.c $(LIBS)
	
testfilebinario:
	gcc -o TestBinario -g testBinario.c $(LIBS)

testopenerrors:
	gcc -o TestOpenErrors -g testOpenErrors.c $(LIBS)
