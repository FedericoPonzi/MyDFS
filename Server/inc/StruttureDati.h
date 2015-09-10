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
	unsigned long int ptid;
	int modo;
	int filesize;
	int socketId; 
	struct OpenedFile* next;
	int transferSockId; /** Usata dall' hearbeating +++++ */ 
	FILE* fp;
}OpenedFile;

/**
 * Questo mutex e' da usare su ogni lavoro che si esegue con la lista linkata di elementi.
 */
extern pthread_mutex_t *mutex; 

extern OpenedFile **openedFileLinkedList;

extern OpenedFile **free_head;

int appendOpenedFile(char* nomefile, int modo);
int fileAlreadyOpenedInWrite(char* filename);
int isModoApertura(int modo_client, int modo);
int checkModoOpen(char *nomeFile , int modo);
void closeOpenedFile(unsigned long int ptid);
void freeOpenedFile(OpenedFile* id);
int getTransferSocket();
char* getFileName();
OpenedFile* getOpenedFile();
int countOpenedFile();

void closeClientSession(unsigned long int ptid);

#endif
