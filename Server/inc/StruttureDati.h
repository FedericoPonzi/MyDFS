#ifndef STRUTTUREDATI
#define STRUTTUREDATI
#define FALSE 0
#define TRUE !(FALSE)
#include <stdio.h>

typedef enum { MYO_RDONLY, MYO_WRONLY, MYO_RDWR, MYO_CREAT, MYO_TRUNC, MYO_EXCL, MYO_EXLOCK} modoApertura_t;

typedef struct OpenedFile
{
	char* fileName;
	int socketId;
	modoApertura_t modo; /** @brief puo' essere un tipo modoApertura **/
	struct OpenedFile* next;
}OpenedFile;

extern OpenedFile* openedFileLinkedList;

void appendOpenedFile(char* nomefile, int modo, int socket);

#endif
