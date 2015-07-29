#include <stdio.h>
#include "inc/Utils.h"
#include "inc/FileHandler.h"
#include "Config.h"
#include "inc/OPEN.h"
#include "inc/CLOSE.h"
#include "inc/READ.h"
#include "inc/WRITE.h"
#include "inc/Utils.h"
int main(int argc, char* argv[])
{
	
	char* indirizzo;
	int error, modo;
	MyDFSId* fileDescr;
	char* file = "file.txt";
	indirizzo = "127.0.0.1";
	modo = 2;
	if(argc == 2)
	{
		file = argv[1];
	}
/*	if(argc == 3)
	{
		modo = argv[2];
	}
	if(argc == 4)
	{
		indirizzo = argv[3];
	}
*/
	error = 0;
	
	printf("\n\tHELLO, CLIENT \n");
	
	
	fileDescr = mydfs_open(indirizzo, file, MYO_CREAT | MYO_RDONLY, &error);
	
	if(error != 0)
	{
		printf("Errore D:");
	}
	mydfs_close(fileDescr);
	return 0;
}

