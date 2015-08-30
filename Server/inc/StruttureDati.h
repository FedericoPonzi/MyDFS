#ifndef STRUTTUREDATI
#define STRUTTUREDATI
#define FALSE 0
#define TRUE !(FALSE)
#include <stdio.h>

//typedef enum { MYO_RDONLY, MYO_WRONLY, MYO_RDWR, MYO_CREAT, MYO_TRUNC, MYO_EXCL, MYO_EXLOCK} modoApertura_t;

typedef struct SocketIdList //struct per lista socket attive
{
	int socketId;
	struct SocketIdList* next;
	int modo;
}SocketIdList;

typedef struct OpenedFile
{
	char* fileName;
	int ptid;
	int modo;
	int filesize;
	struct OpenedFile* next;
	int transferSockId;
	FILE* fp;
}OpenedFile;


extern pthread_mutex_t *mutex;

extern OpenedFile **openedFileLinkedList;

extern OpenedFile **free_head;

int appendOpenedFile(char* nomefile, int modo);
int fileAlreadyOpenedInWrite(char* filename);
int isModoApertura(int modo_client, int modo);
int checkModoOpen(char *nomeFile , int modo);
int getTransferSocket();
char* getFileName();
OpenedFile* getOpenedFile();

void closeClientSession(int ptid);

#endif
