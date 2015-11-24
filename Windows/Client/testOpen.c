#include <stdio.h>
#include "inc/Utils.h"
#include "inc/Config.h"
#include "inc/OPEN.h"
#include "inc/CLOSE.h"
#include "inc/READ.h"
#include "inc/WRITE.h"
#include "inc/Utils.h"
#include "assert.h"
#include <string.h>
#include "inc/Test.h"


void singleOpen(char * filename, char* indirizzo, int lel)
{

    printf("[TEST OPEN] Iniziato \n");

	/*
	 * Modalita' delle open:
	 */	

	MyDFSId* fileId;
	int error = 0;
	fileId = mydfs_open(indirizzo, filename, MYO_CREAT, &error);
    assert(fileId != NULL && error == 0);
		
    mydfs_close(fileId);
	printf("\n\t[V-%d] Test OPEN superato correttamente!\n", getpid());
}

int main(int argc, char* argv[])
{
	char* filename;
	
	if(argc < 2)
	{
		filename = "file.txt";
	}
	else
	{
		filename=argv[1];
	}
    char* indirizzo = "127.0.0.1";
    
    printf("Sizeof(Handle): %lu, sizeof(pvoid): %lu", sizeof(HANDLE), sizeof(PVOID));
    testOpen(filename, indirizzo, 1 );

	return 0;	
}
