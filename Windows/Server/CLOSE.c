#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include "inc/OPE.h"
#include "inc/StruttureDati.h"
#include "inc/Utils.h"
#include "inc/CLOSE.h"
#include "inc/Config.h"


int getChunkPosition(char* buffer);
int getChunkSize(char*);

int getNumberOfChanges(char* command);
int handleWrites(int numberOfChanges, OpenedFile* id);
void sendInvalidate(OpenedFile* id);
/**
 * @brief Gestisce il comando Close.
 * @param int socket socket del client che termina sessione
 * Ci sono casi in cui questa funzione non viene richiamata, ad esempio se l' heartbeating chiude la connessione da solo.
 * In quel caso, id sara' null.
 */
void handleCloseCommand(char* command, int socket)
{	
	OpenedFile* id = getOpenedFile();
	if(id != NULL && ( isModoApertura(id->modo, MYO_WRONLY) || isModoApertura(id->modo, MYO_RDWR)))
	{	
		handleWrites(getNumberOfChanges(command), id);
		sendInvalidate(id);
	}
    if(procOrThread)
    {
        UnmapViewOfFile(hSharedMemory);
        CloseHandle(hSharedMemory);
    }
    shutdown(id->controlSocketId, SD_BOTH);

}

/**
 * @brief gestisce tutte le writes.
 */
int handleWrites(int numberOfChanges, OpenedFile* id)
{
	printf("HandleWrites: %d numero di cambiamenti. \n", numberOfChanges);
	int i = 0, nRecv = 0;
	char messaggio[100];
	while(i < numberOfChanges)
	{
		nRecv = recv (id->controlSocketId, messaggio, sizeof(messaggio), 0);
		if(nRecv < 0)
		{
			perror("Error on recv.");
			return -1;
		}
		else if(nRecv == 0)
		{
			printf("Il peer e' andato.");
			return 0;
		}

		//Mi prendo la quantita e la posizione:
		int size = getChunkSize(messaggio);
		int pos = getChunkPosition(messaggio);
		
		//Preparo il file pointer:
		FILE* fp = id->fp;

		fseek(fp, pos, SEEK_SET);

		//Nel caso sia una grossa modifica, spezziamo in più writes
		while(size > 0)
		{
			printf("Ricevo dati:\n");
			int buffSize = size > FILESIZE ? FILESIZE : size;
			char* buffer = malloc(buffSize);
			nRecv = recv(id->controlSocketId, buffer, buffSize, 0);
			if(nRecv < 0)
			{
				perror("Recv: ");
				return -1;
			}
			printf("Messaggio size: '%d', in posizione: %lu, buff: '%s'\n", nRecv, ftell(fp), buffer);
            
			int w = fwrite(buffer, 1, nRecv, fp);
			printf("Scritti %d dati nel file. \n", w);
			free(buffer);
			size -= nRecv;
		}
		//Aggiorno la dimensione del file:
		if(pos >= id->filesize)
		{
			id->filesize = id->filesize + size;
		}
		i++;
		
	}
	
	return 0;
}


/**
 * Ritorna il numero di cambiamenti n specificato dal comando CLO n
 */
int getNumberOfChanges(char* command)
{
	int toRet;
	toRet = strtol(command+strlen(CLOSECOMMAND)+1, 0, 10); // "CLOS 0" close e' lungo 5.
	return toRet;
}

/**
 * @brief Prende in input il comando, ed estrare la posizione della write.
 * es: "POS 0 SIZE 1000" torna 0.
 */
int getChunkPosition(char* buffer)
{
	int toRet;
	toRet = strtol(buffer + strlen("POS "), 0, 10);
	return toRet;
}

/**
 * @brief Prende in input il comando, ed estrare la dimensione della write.
 * es: "POS 0 SIZE 1000" torna 1000.
 */
int getChunkSize(char* buffer)
{
	int toRet;
	int i = 0;

	while(strncmp(buffer+i, "SIZE ", strlen("SIZE ")) != 0) i++;
	i+= strlen("SIZE ");
	toRet = strtol(buffer + i, 0, 10);
	return toRet;
}

/** 
 * @brief Invia il messaggio di invalidazione ai client che hanno il file aperto in lettura.
 */
void sendInvalidate(OpenedFile* id)
{
	printf("[Close] Cerco client per messaggio di Invalidazione. Mio ptid: %lu \n", id->ptid);
	WaitForSingleObject(mutex, INFINITE);

	OpenedFile* iterator = openedFileLL;
	while(iterator != NULL)
	{
		if(iterator->used == 1 && strncmp(iterator->fileName, id->fileName, strlen(INVALIDATECOMMAND)) == 0)
		{
			if(iterator->ptid != id->ptid)
			{
				printf("[Close] Trovato client, gli invio il comando\n");
				printf("Socketid:%d, processo: %lu\n", iterator->controlSocketId, iterator->ptid);
                //Se e' un processo
                if(procOrThread)
                {
                    char pipeName[100];
                    sprintf(pipeName, "%s%lu",PIPEPREFIX, iterator->ptid);
                    HANDLE hPipe = INVALID_HANDLE_VALUE;
                    logM("pipename: %s\n", pipeName);
                    while((hPipe = CreateFile(
                         pipeName,   // pipe name
                         GENERIC_READ |  // read and write access
                         GENERIC_WRITE,
                         0,              // no sharing
                         NULL,           // default security attributes
                         OPEN_EXISTING,  // opens existing pipe
                         0,              // default attributes
                         NULL)) == INVALID_HANDLE_VALUE)
                    {
                        logM    ("Nop");
                        Sleep(100);

                        //Aspetto che venga creata la pipe.
                    }

                    if( WriteFile(hPipe, INVALIDATECOMMAND, sizeof(INVALIDATECOMMAND), NULL, NULL) == 0 )
                    {
                        printf("non sto scrivendo indirizzo mutex\n");
                        //exit(EXIT_FAILURE);
                    }
                    CloseHandle(hPipe);

                }
                else
                {
                    WaitForSingleObject(id->controlSocketMutex, INFINITE );
                    if(send(iterator->controlSocketId, INVALIDATECOMMAND, strlen(INVALIDATECOMMAND), 0) < 0)
                    {
                        perror("Error sending INVA\n");
                    }
                    ReleaseMutex(&iterator->controlSocketMutex);
                }
			}
		}
		iterator = (iterator->nextOffset > 0)? iterator+1 : NULL;
	}
	ReleaseMutex(mutex);
}
