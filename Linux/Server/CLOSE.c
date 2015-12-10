#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <errno.h>
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
	logM("[handleWrites] %d numero di cambiamenti. \n", numberOfChanges);
	int i = 0, nRecv = 0;
	char messaggio[100];
	while(i < numberOfChanges)
	{
		nRecv = recv (id->controlSocketId, messaggio, sizeof(messaggio), 0);
		if(nRecv < 0)
		{
			perror("[handleWrites] Error on recv.");
			return -1;
		}
		else if(nRecv == 0)
		{
			logM("[handleWrites] Il peer e' andato.");
			return 0;
		}

		//Mi prendo la quantita e la posizione:
		int size = getChunkSize(messaggio);
		int pos = getChunkPosition(messaggio);
        if(size< 0 || pos < 0)
        {
            logM("[handleWrites] Error receving chunk size(%d) or position(%d).", size, pos);
            return -1;
        }
		
		//Preparo il file pointer:
		FILE* fp = id->fp;

		if(fseek(fp, pos, SEEK_SET))
        {
            perror("[handleWrites] Fseek:");
            return -1;
        }
        

		//Nel caso sia una grossa modifica, spezziamo in più writes
		while(size > 0)
		{
			logM("[handleWrites] Ricevo dati:\n");
			int buffSize = size > FILESIZE ? FILESIZE : size;
			char* buffer = malloc(buffSize);
            if(buffer == NULL)
            {
                perror("[handleWrites] Malloc:");
                return -1;
            }
			nRecv = recv(id->controlSocketId, buffer, buffSize, 0);
			if(nRecv < 0)
			{
				perror("[handleWrites] Recv: ");
				return -1;
			}
			logM("[handleWrites] Messaggio size: '%d', in posizione: %lu, buff: '%s'\n", nRecv, ftell(fp), buffer);
            
			int w = fwrite(buffer, 1, nRecv, fp);
			logM("[handleWrites] Scritti %d dati nel file. \n", w);
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
 * @brief Ritorna il numero di cambiamenti n dal comando CLOS n
 * @return n numero di cambiamenti
 * @return -1 se strtol e' fallita
 */
int getNumberOfChanges(char* command)
{
	
	int toRet;
	toRet = strtol(command+4, 0, 10); // "CLO 0" close e' lungo 5.
    if(errno == EINVAL || errno == ERANGE)
    {
        toRet = -1;
    } 
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
    if(errno == EINVAL || errno == ERANGE)
    {
        toRet = -1;
    }
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
    if(errno == EINVAL || errno == ERANGE)
    {
        toRet = -1;
    }
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
				//Se e' un thread
                if(procOrThread)
                {
                    int i;
                    i = kill(iterator->ptid, SIGUSR1);
                    if(i < 0) perror("Impossibile mandare segnale inva.\n");
                    logM("Mandato segnale!\n");
                }
                else
                {
                    pthread_mutex_lock(&iterator->controlSocketMutex);
                    if(send(iterator->controlSocketId, invalidate, strlen(invalidate), 0) < 0)
                    {
                        perror("Error sending INVA\n");
                    }
                    pthread_mutex_unlock(&iterator->controlSocketMutex);
                }
			}
		}
		iterator = iterator->next;
	}
	pthread_mutex_unlock(mutex);
}
