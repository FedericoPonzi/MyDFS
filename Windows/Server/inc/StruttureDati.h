#ifndef STRUTTUREDATI_T
#define STRUTTUREDATI_T

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
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
	SOCKET transferSocketId; 
	SOCKET controlSocketId; /** Usata dall' hearbeating +++++ */
    HANDLE controlSocketMutex; /** Mutex della socket dell' heartbeating */
    long unsigned int nextOffset;
    struct OpenedFile* next;
    FILE* fp;
    int used; //Booleano che dice se il nodo e' in uso
}OpenedFile;

/**
 * Questo mutex e' da usare su ogni lavoro che si esegue con la lista linkata di elementi.
 */
extern HANDLE mutex; 

extern OpenedFile **openedFileLinkedList;

extern OpenedFile **free_head;

OpenedFile* openedFileLL;


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
