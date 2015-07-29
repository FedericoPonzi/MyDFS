#ifndef STRUTTUREDATI
#define STRUTTUREDATI
#define FALSE 0
#define TRUE !(FALSE)
#include <stdio.h>

//typedef enum { MYO_RDONLY, MYO_WRONLY, MYO_RDWR, MYO_CREAT, MYO_TRUNC, MYO_EXCL, MYO_EXLOCK} modoApertura_t;

#define MYO_RDONLY 1
#define MYO_WRONLY 2
#define MYO_RDWR   4
#define MYO_CREAT  8
#define MYO_TRUNC  16
#define MYO_EXCL   32
#define MYO_EXLOCK 64

typedef struct SocketIdList //struct per lista socket attive
{
	int socketId;
	struct SocketIdList* next;
	int modo;
}SocketIdList;

typedef struct OpenedFile
{
	char* fileName;
	int socketId;
	int modo;
	struct OpenedFile* next;
}OpenedFile;


extern OpenedFile **openedFileLinkedList;
extern OpenedFile **free_head;

void allocaEInizializzaMemoria();
int appendOpenedFile(char* nomefile, int modo, int socket);
int fileAlreadyOpenedInWrite(char* filename, int socketId);
int isModoApertura(int modo_client, int modo);


void closeClientSession(int sd /*, char* fileName*/);

#endif
