#ifndef STRUTTUREDATI
#define STRUTTUREDATI
#define FALSE 0
#define TRUE !(FALSE)
#include <stdio.h>

typedef struct OpenedFile
{
	char* fileName;
	int socketId;
	int modo; /** @brief puo' essere uno fra O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_TRUNC, O_EXCL,  O_EXLOCK **/
	struct OpenedFile* next;
	
}OpenedFile;

extern OpenedFile* openedFileLinkedList;

void appendOpenedFile(char* nomefile, int modo, int socket);

#endif
