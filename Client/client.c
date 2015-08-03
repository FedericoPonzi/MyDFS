#include <stdio.h>
#include "inc/Utils.h"
#include "inc/FileHandler.h"
#include "Config.h"
#include "inc/OPEN.h"
#include "inc/CLOSE.h"
#include "inc/READ.h"
#include "inc/WRITE.h"
#include "inc/Utils.h"
#include "assert.h"

int main(int argc, char* argv[])
{
	
	char* indirizzo;
	int error, modo;
	MyDFSId* fileId;
	MyDFSId* fileId2;
	char* filename = "file.txt";
	char* fileInesistente;
	int array[] = { MYO_RDONLY, MYO_WRONLY, MYO_RDWR, MYO_CREAT, MYO_TRUNC, MYO_EXCL, MYO_EXLOCK};
	indirizzo = "127.0.0.1";
	
	if(argc == 2)
	{
		filename = argv[1];
	}

	error = 0;
	
	printf("\n\tHELLO, CLIENT \n");
	printf("[TEST DELLA OPEN]:\n");

	int z = 0;
	int i;
	for(i = 0; i < 7; i++)
	{
		int k;
		for(k =0; k < 1000; k++)
		{
			printf("\n * Modo numero: %d \n", array[i]);
			fileId = mydfs_open(indirizzo, filename, array[i], &error);
			if(error != 0)
			{
				printf("Errore %d\n", error);
				assert(0);
			}
			mydfs_close(fileId);
			z++;
			printf("z: %d\n", z);
		}
	}
	
	
	return 0;
}

