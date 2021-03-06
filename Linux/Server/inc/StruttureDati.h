#ifndef STRUTTUREDATI
#define STRUTTUREDATI
#define FALSE 0
#define TRUE !(FALSE)
#include <stdio.h>

//typedef enum { MYO_RDONLY, MYO_WRONLY, MYO_RDWR, MYO_CREAT, MYO_TRUNC, MYO_EXCL, MYO_EXLOCK} modoApertura_t;

typedef struct SocketIdList //struct per lista socket attive
{
	int transferSocketId;
	struct SocketIdList* next;
	int modo;
}SocketIdList;

typedef struct OpenedFile
{
	char fileName [FILENAME_MAX]; 
	unsigned long int ptid;
	int modo;
	int filesize;
	int transferSocketId; 
	struct OpenedFile* next;
	int controlSocketId; /** Usata dall' hearbeating +++++ */
    pthread_mutex_t controlSocketMutex;
	FILE* fp;
}OpenedFile;

/**
 * Questo mutex e' da usare su ogni lavoro che si esegue con la lista linkata di elementi.
 */
extern pthread_mutex_t *mutex; 

extern OpenedFile **openedFileLinkedList;

extern OpenedFile **free_head;

int appendOpenedFile(char* nomefile, int modo, OpenedFile** id );
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
