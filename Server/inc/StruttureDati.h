#ifndef STRUTTUREDATI
#define STRUTTUREDATI
#define FALSE 0
#define TRUE !(FALSE)

typedef struct OpenedFile
{
	char* fileName;
	int socketId;
	int modo;
	struct OpenedFile* next;
}OpenedFile;

extern OpenedFile* openedFileLinkedList;

void appendOpenedFile(char* nomefile, int modo, int socket);

#endif
