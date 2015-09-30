#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
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
	closeClientSession(getptid());
	close(socket);	
}

/**
 * @brief gestisce tutte le writes.
 */
int handleWrites(int numberOfChanges, OpenedFile* id)
{
	logM("HandleWrites: %d numero di cambiamenti. \n", numberOfChanges);
	int i = 0, n = 0;
	char messaggio[100];
	while(i < numberOfChanges)
	{
		n = recv (id->transferSockId, messaggio, sizeof(messaggio), 0);
		if(n < 0)
		{
			perror("Error on recv.");
			return -1;
		}
		else if(n == 0)
		{
			logM("Il peer e' andato.");
			return 0;
		}
		logM("Messaggio: %d dati,  '%s'\n", n,  messaggio);
		//Mi prendo la quantita e la posizione:
		int size = getChunkSize(messaggio);
		int pos = getChunkPosition(messaggio);
		
		//Preparo il file pointer:
		FILE* fp = id->fp;
		fseek(fp, pos, SEEK_SET);

		//Nel caso sia una grossa modifica, spezziamo in più writes
		while(size > 0)
		{
			logM("Ricevo dati:\n");
			int buffSize = size > FILESIZE ? FILESIZE : size;
			char* buffer = malloc(buffSize); /**@todo : cambiare a void*/
			n = recv(id->transferSockId, buffer, buffSize, 0);
			logM("Messaggio size: '%d', '%s'\n", n, buffer);
			
			if(n < 0)
			{
				perror("Recv: ");
				return -1;
			}
			logM("Buffer: %s %lu\n", buffer, ftell(fp));
			int w = fwrite(buffer, 1, n, fp);
			logM("Scritti %d dati nel file. \n", w);
			free(buffer);
			size -= n;
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


int getNumberOfChanges(char* command)
{
	
	int toRet;
	toRet = strtol(command+4, 0, 10); // "CLO 0" close e' lungo 5.
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
	logM("[Close] Cerco client per messaggio di Invalidazione. Mio ptid: %lu \n", id->ptid);
	pthread_mutex_lock(mutex);
	char* invalidate = "INVA";
	OpenedFile* iterator = *openedFileLinkedList;
	while(iterator != NULL)
	{
		if(strcmp(iterator->fileName, id->fileName) == 0)
		{
			if(iterator->ptid != id->ptid)
			{
				logM("[Close] Trovato client, gli invio il comando\n");
				logM("Socketid:%d, processo: %lu\n", iterator->transferSockId, iterator->ptid);
                //Se e' un thread
                if(procOrThread)
                {
                    int i;
                    i = kill(iterator->ptid, SIGUSR1);
                    if(i < 0) perror("Impossibile mandare segnale inva.\n");
                    logM("Mandato segnale!");
                }
                else
                {
                    pthread_mutex_lock(tempSockMutex);
                    if(send(iterator->transferSockId, invalidate, strlen(invalidate), 0) < 0)
                    {
                        perror("Error sending INVA\n");
                    }
                    pthread_mutex_unlock(tempSockMutex);                }
			}
		}
		iterator = iterator->next;
	}
	pthread_mutex_unlock(mutex);
}
