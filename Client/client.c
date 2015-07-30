#include <stdio.h>
#include "inc/Utils.h"
#include "inc/FileHandler.h"
#include "Config.h"
#include "inc/OPEN.h"
#include "inc/CLOSE.h"
#include "inc/READ.h"
#include "inc/WRITE.h"
#include "inc/Utils.h"

void swap(char **x, char **y)
{
    char* temp;
    temp = *x;
    *x = *y;
    *y = temp;
}
 
void permute(char *a[], int l, int r)
{
   int i;
   if (l == r)
   {
		for(i = 0; i < l; i++)
		{
			 printf("%s, ", a[i]);
		}
			printf("\n");
	}

   else
   {
       for (i = l; i < r; i++)
       {
          swap((&a[l]), (&a[i]));
          permute(a, l+1, r);
          swap(&a[l], &a[i]); //backtrack
       }
   }
}

int main(int argc, char* argv[])
{
	
	char* indirizzo;
	int error, modo;
	MyDFSId* fileId;
	MyDFSId* fileId2;
	char* filename = "file.txt";
	indirizzo = "127.0.0.1";
	modo = 2;
	if(argc == 2)
	{
		file = argv[1];
	}

	error = 0;
	
	printf("\n\tHELLO, CLIENT \n");
	printf("TEST DELLA OPEN:\n");
	
	printf(" * MYO_WRONLY\n");
	fileId = mydfs_open(indirizzo, filename, MYO_WRONLY, &error);
	
	if(error != 0)
	{
		printf("Errore D:");
		assert(0);
	}
	
	mydfs_close(fileId);

	printf(" * MYO_CREAT | MYO_WRONLY\n");
	
	fileId = mydfs_open(indirizzo, file, MYO_CREAT | MYO_WRONLY, &error);
	mydfs_close(fileId);
	
	if(error != 0)
	{
		printf("Errore D:");
	}

	printf(" * MYO_CREAT | MYO_WRONLY | MYO_TRUNC\n");
	
	fileId = mydfs_open(indirizzo, file, MYO_CREAT | MYO_RDWR, &error);
	mydfs_close(fileId);
	
	if(error != 0)
	{
		printf("Errore D:");
	}
	return 0;
}

