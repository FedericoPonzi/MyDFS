#include <stdio.h>
#include <string.h>
#include "second.h"
#include <stdlib.h> 
#include "terzo.h"
OpenedFile* mainFile;

int main()
{
	mainFile= (OpenedFile*) malloc(sizeof(OpenedFile));
	mainFile->fileName = "Ciao";
	mainFile->next = NULL;
	append("File");
	append("Lol");
	append("ciao");
	if(fileAlreadyOpen("Lol"))
	{
		printf("gia' aperto sorry! :C\n");
	}
	else
	{
		printf("Aperto con successo :D\n");
	}
	
	return 0;

}
