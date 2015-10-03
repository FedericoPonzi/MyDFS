#include "second.h"


void append(char* nome)
{
	OpenedFile* iterator = mainFile;
	printf("YO\n");
	while(iterator->next != NULL)
	{
		iterator = iterator->next;
	}
	OpenedFile* prova;
	prova = (OpenedFile*) malloc(sizeof(OpenedFile));
	prova->fileName = nome; 
	prova->next =NULL;
	iterator->next = prova;

}

int fileAlreadyOpen(char* fileName)
{
	OpenedFile* iterator = mainFile;
	printf("YO\n");
	while(iterator->next != NULL)
	{
		if(strcmp(iterator->fileName, fileName) == 0)
		{
			return TRUE;
		}
		printf("Provo: '%s' \n", iterator->fileName);
		iterator = iterator->next;
	}
	return FALSE;
	
}
