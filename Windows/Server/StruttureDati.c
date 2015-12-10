
/**
 * @file StruttureDati.c
 * @brief Contiene le funzioni di utilita' per le Strutture Dati
 * 
 * @todo Una struttura dati che contenga una lista di sockets
 * @todo Remove e add di una socket nella lista di sockets, remove di un file dalla lista di file aperti.
 * 
 * Contiene tutte le funzioni utili per lavorare con le strutture dati definite nell' header di questo modulo.
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include "inc/Heartbeating.h"
#include "inc/Config.h"
#include "inc/Utils.h"
#include "inc/StruttureDati.h"
#include "inc/OPE.h"

HANDLE mutex;

OpenedFile* openedFileLL;


/**
 * @brief Aggiunge l' oggetto in input alla fine della lista linkata dei file aperti OpenedFile
 * 
 * Se non esiste la radice mainFile la crea. Altrimenti scorre tutti i nodi fino ad arrivare all' elemento in coda, crea un nuovo
 * nodo con i dati in input e lo appende alla fine.
 */
int appendOpenedFile(char* nomeFile, int modo, OpenedFile** id)
{
	printf("[appendOpenedFile] - Appendo il file aperto.\n");
	//Se richiede una scrittura, e il file e' già aperto in scrittura:
	if(fileAlreadyOpenedInWrite(nomeFile)) //Aperto in scrittura
	{
		printf("File gia aperto in scrittura!\n");
		return 1;
	}

	if(checkModoOpen(nomeFile, modo))
	{
		printf("CheckModoOpen error\n");
		return -3;
	}

	WaitForSingleObject(mutex, INFINITE);
    
    OpenedFile* n = openedFileLL;
    while(n->nextOffset > 0 && n->used != 0 )
    {
        n++;
    }
    //Se sono arrivato alla fine della lista e l' ultimo oggetto e' in uso, ho finito gli slots.
    if (n->used == 1) 
	{
		ReleaseMutex(mutex);
		printf("StrutturaDati: n e' null.\n");
		numberAliveChilds--;
		myExit();
	}
	char filePath[strlen(nomeFile) + strlen(rootPath)+1];
	sprintf(filePath, "%s%s", rootPath,nomeFile);
    strcpy(n->fileName, nomeFile);
    
    n->used = 1;
    n->ptid = getptid();
	n->modo = modo;
    n->controlSocketMutex = CreateMutex(NULL, FALSE, NULL);
    
	//Il controllo per vedere se il file esiste e tornare errore in caso, lo faccio prima
    
	if(isModoApertura(modo, MYO_CREAT) && isModoApertura(modo, MYO_TRUNC))
	{
		n->fp = fopen(filePath, "w+b");
	}
	else
    {
        if(isModoApertura(modo, MYO_CREAT))
        {
            n->fp = fopen(filePath, "a");
            fclose(n->fp);
        }
        n->fp = fopen(filePath, "r+b");
    }
	ReleaseMutex(mutex);
    *id = n;
    return 0;
}

/**
 * @brief controlla se la open è realizzabile con il modo richiesti
 */
int checkModoOpen(char *nomeFile, int modo)
{

	
	//copio in temp_path il rootPath
	char temp_path[FILENAME_MAX];
	strcpy(temp_path, rootPath);
	strcat(temp_path, nomeFile);

	if(access(temp_path, F_OK) == -1)
	{
		if(!isModoApertura(modo, MYO_CREAT))
		{
			printf("File inesistente");			
			return -3;
		}
	}
	else
	{
		if(isModoApertura(modo, MYO_EXCL))
		{            
            printf("MYO_EXCL di un file esistente.");
			return -3;
		}
	}

	OpenedFile* iterator = openedFileLL;

    WaitForSingleObject(mutex, INFINITE);
	while(iterator != NULL)
	{
		if(iterator->used && strcmp(iterator->fileName, nomeFile) == 0)
		{
			if(
			isModoApertura(iterator->modo, MYO_WRONLY) || 
			isModoApertura(iterator->modo, MYO_RDWR) || 
			isModoApertura(iterator->modo, MYO_EXLOCK))
			{
				ReleaseMutex(mutex);

				return -3;
			}
		}
		iterator = (iterator->nextOffset > 0)? iterator+1 : NULL;

	}
	ReleaseMutex(mutex);

	
	return 0;
}

/**
 * @brief Controlla se un file con quel nome e' gia' stato aperto in una modalita' che non permette l'accesso da parte di altri client
 * NOTA: SINCRONIZZATO CON MUTEX
 */

int fileAlreadyOpenedInWrite(char* filename)
{
	WaitForSingleObject(mutex, INFINITE);
		
	OpenedFile* iterator = openedFileLL;
	while(iterator != NULL)
	{
		if(iterator->used && strcmp(iterator->fileName, filename) == 0)
		{
			if(isModoApertura(iterator->modo, MYO_EXLOCK) || isModoApertura(iterator->modo, MYO_WRONLY) || isModoApertura(iterator->modo, MYO_RDWR))
			{
				ReleaseMutex(mutex);
				printf("iterator->filename = %s\n", iterator->fileName);
				return 1;
			}
		}
		iterator = (iterator->nextOffset > 0)? iterator+1 : NULL;

	}
	ReleaseMutex(mutex);
	
	printf("[fileAlreadyOpenedInWrite] - Nessun file aperto con questo nome in scrittura: '%s' \n",filename);

	return 0;
}


/**
 * @brief Dice se modo_client contiene il modo "modo".
 * 
  */
int isModoApertura(int modo_client, int modo)
{
	if((modo_client&modo)== modo)
	{
		return TRUE;
	}
	return FALSE;
}


/**
 * Libera lo spazio occupato dal puntatore

 */
void freeOpenedFile(OpenedFile* id)
{
	fclose(id->fp);
    logM("[FreeOpenedfile] Libero: transfer: %d, sock: %d\n", id->controlSocketId, id->transferSocketId);
    
    //Se al momento della OPE ho creato l' heartbeating, eseguo solo lo shutdown della socket e lascio la closesocket all' heartbeating
    if((isModoApertura(modo, MYO_WRONLY) || isModoApertura(modo, MYO_RDWR)))
    {
        shutdown(id->controlSocketId, SD_BOTH);
        shutdown(id->transferSocketId, SD_BOTH);
    }
    else
    {
        //Chiudo l' handle del mutex, se non c'e' il thread dell' heartbeating.
        CloseHandle(id->controlSocketMutex);
        closesocket(id->controlSocketId);
        closesocket(id->transferSocketId);
    }
   
    //Se sono in modalita multiprocesso, chiudo le winsock.
    if(procOrThread)
    {
        WSACleanup();
    }
    
    long unsigned int nextOffset = id->nextOffset;
	ZeroMemory(id, sizeof(OpenedFile));
    id->nextOffset = nextOffset;
    id->used = 0;

}


/**
 * @brief rimuove collegamenti tra client e file aperti nella sessione
 * NOTA: ACCESSO SINCRONIZZATO CON MUTEX
 * @param int sd socket descriptor del client
 */
void closeClientSession(unsigned long int ptid) 
{
	WaitForSingleObject(mutex, INFINITE);
	
	OpenedFile* iterator = openedFileLL;

    while(iterator != NULL)
    {
        if (iterator->used && iterator->ptid == ptid)
        {
            freeOpenedFile(iterator);
        }
		iterator = (iterator->nextOffset > 0) ? iterator+1 : NULL;

    }
	ReleaseMutex(mutex);
	printf("[CloseClientSession] Numero connessioni rimaste: %d\n", countOpenedFile());
	printf("[closeClientSession] - Connessione chiusa.\n");
}

/**
 * Debug
 */
int getTransferSocket()
{
	WaitForSingleObject(mutex, INFINITE);

	OpenedFile* iterator = openedFileLL;
	while(iterator != NULL)
	{
		if(iterator->used && iterator->ptid == getptid())
		{
			ReleaseMutex(mutex);
			return iterator->controlSocketId;
		}
		iterator = (iterator->nextOffset > 0)? iterator+1 : NULL;

	}
	ReleaseMutex(mutex);
	return 0;
}
/**
 * @brief Ritorna il nome del file aperto da questo ptid.
 * NOTA: ACCESSO SINCRONIZZATO CON MUTEX
 */
char* getFileName()
{
	WaitForSingleObject(mutex, INFINITE);
	
	OpenedFile* iterator = openedFileLL;
	while(iterator != NULL)
	{
		if(iterator->used && iterator->ptid == getptid())
		{
			ReleaseMutex(mutex);
			return iterator->fileName;
		}
		iterator = (iterator->nextOffset > 0)? iterator+1 : NULL;

	}
	ReleaseMutex(mutex);
	return NULL;
}

/**
 * @brief Ritorna l' opened file associato a questo ptid
 */
OpenedFile* getOpenedFile()
{
	WaitForSingleObject(mutex, INFINITE);
    long unsigned ptid = getptid();
	OpenedFile* iterator = openedFileLL;
	while(iterator != NULL)
	{
		if(iterator->used && iterator->ptid == ptid)
		{
			ReleaseMutex(mutex);
			return iterator;
		}
		iterator = (iterator->nextOffset > 0)? iterator+1 : NULL;

	}
	
	ReleaseMutex(mutex);
	return NULL;
}

/**
 * Ritorna il numero di righe della tabella con i file aperti.
 */
int countOpenedFile()
{
	int i = 0;
	WaitForSingleObject(mutex, INFINITE);

	OpenedFile* iterator = openedFileLL;
	while(iterator != NULL)
	{
        if(iterator->used == 1) i++;
		iterator = (iterator->nextOffset > 0) ? iterator+1 : NULL;

	}
	ReleaseMutex(mutex);
	return i;
}
