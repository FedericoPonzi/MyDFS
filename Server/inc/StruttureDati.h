#ifndef STRUTTUREDATI
#define STRUTTUREDATI

typedef struct OpenedFile
{
	char* filename;
	int socketId;
	int modo;
	struct OpenedFile* next;
}OpenedFile;

extern OpenedFile* mainFile;


#endif
