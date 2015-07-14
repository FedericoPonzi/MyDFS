#ifndef second
#define second
#include <stdio.h>
#include <stdlib.h> 
#include <stdio.h>

#define FALSE 0
#define TRUE !(FALSE)

typedef struct OpenedFile
{
	char* fileName;
	
	 struct OpenedFile* next;
	
}OpenedFile;

extern OpenedFile* mainFile;
int fileAlreadyOpen(char* fileName);

void append();
#endif
