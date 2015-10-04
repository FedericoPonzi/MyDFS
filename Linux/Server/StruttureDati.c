/*
 * 
 * Copyright 2015 Federico Ponzi <federicoponzi@federicos-ubuntu>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * CHANGELOG:
 * 	11.07.2015: Creazione del file.
 * 
 */
/**
 * @file StruttureDati.c
 * @author Federico Ponzi
 * @date 11.07.2015
 * @brief Contiene le funzioni di utilita' per le Strutture Dati
 * 
 * @todo Una struttura dati che contenga una lista di sockets
 * @todo Remove e add di una socket nella lista di sockets, remove di un file dalla lista di file aperti.
 * NOTA: Ricordarsi di eseguire una free() dei OpenedFile->fileName perchè viene allocata con una malloc in OPE.c
 * 
 * Contiene tutte le funzioni utili per lavorare con le strutture dati definite nell' header di questo modulo.
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include "inc/Heartbeating.h"
#include "inc/Config.h"
#include "inc/Utils.h"
#include "inc/StruttureDati.h"
#include "inc/OPE.h"


OpenedFile ** openedFileLinkedList = NULL;
OpenedFile ** free_head;
pthread_mutex_t *mutex;



/**
 * @brief Aggiunge l' oggetto in input alla fine della lista linkata dei file aperti OpenedFile
 * 
 * Se non esiste la radice mainFile la crea. Altrimenti scorre tutti i nodi fino ad arrivare all' elemento in coda, crea un nuovo
 * nodo con i dati in input e lo appende alla fine.
 */
int appendOpenedFile(char* nomeFile, int modo, OpenedFile** id)
{
	logM("[appendOpenedFile] - Appendo il file aperto.\n");
	//Se richiede una scrittura, e il file e' già aperto in scrittura:
	if(fileAlreadyOpenedInWrite(nomeFile)) //Aperto in scrittura
	{
		logM("File gia aperto in scrittura!\n");
		return 1;
	}

	if(checkModoOpen(nomeFile, modo))
	{
		logM("CheckModoOpen error\n");
		return -3;
	}

	pthread_mutex_lock(mutex);

	OpenedFile *n = *free_head;
	
	if (n == NULL) 
	{
		pthread_mutex_unlock(mutex);
		logM("StrutturaDati: n e' null.\n");
		numberAliveChilds--;
		myExit();
		
	}
	
	char filePath[strlen(nomeFile) + strlen(rootPath)+1];
	sprintf(filePath, "%s%s", rootPath,nomeFile);
	
	*free_head = (*free_head)->next;
	n->fileName = nomeFile;
	n->ptid = getptid();
	n->modo = modo;
    pthread_mutexattr_t mutex_attr;
	
    if (pthread_mutexattr_init(&mutex_attr) < 0) 
    {
        perror("Failed to initialize mutex attributes");
        return -3;
    }
    
    if (pthread_mutex_init(&n->tempSockMutex, &mutex_attr) < 0) 
    {
        perror("Failed to initialize mutex");
        return -3;
    }
    
	n->next = *openedFileLinkedList;
	
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

	*openedFileLinkedList = n;

	pthread_mutex_unlock(mutex);
    *id = n;
	return 0;
}

/**
 * @brief controlla se la open è realizzabile con il modo richiesti
 */
int checkModoOpen(char *nomeFile, int modo)
{
	pthread_mutex_lock(mutex);

	OpenedFile* iterator = *openedFileLinkedList;
	
	//copio in temp_path il rootPath
	char temp_path[40];
	strcpy(temp_path, rootPath);
	strcat(temp_path, nomeFile);

	if(access(temp_path, F_OK) == -1)
	{
		if(!isModoApertura(modo, MYO_CREAT))
		{
			pthread_mutex_unlock(mutex);
			logM("File inesistente");			
			return -3;
		}
	}
	else
	{
		if(isModoApertura(modo, MYO_EXCL))
		{
			pthread_mutex_unlock(mutex);

			return -3;
		}
	}
	while(iterator != NULL)
	{
		if(strcmp(iterator->fileName, nomeFile) == 0)
		{
			if(
			isModoApertura(iterator->modo, MYO_WRONLY) || 
			isModoApertura(iterator->modo, MYO_RDWR) || 
			isModoApertura(iterator->modo, MYO_EXLOCK))
			{
				pthread_mutex_unlock(mutex);

				return -3;
			}
		}
		iterator = iterator->next;
	}
	pthread_mutex_unlock(mutex);

	
	return 0;
}

/**
 * @brief Controlla se un file con quel nome e' gia' stato aperto in una modalita' che non permette l'accesso da parte di altri client
 * NOTA: SINCRONIZZATO CON MUTEX
 */

int fileAlreadyOpenedInWrite(char* filename)
{
	pthread_mutex_lock(mutex);		
	if(openedFileLinkedList == NULL)
	{
		pthread_mutex_unlock(mutex);		
		return FALSE;
	}
	OpenedFile* iterator = *openedFileLinkedList;
	while(iterator != NULL)
	{
		if(strcmp(iterator->fileName, filename) == 0)
		{
			if(isModoApertura(iterator->modo, MYO_EXLOCK) || isModoApertura(iterator->modo, MYO_WRONLY) || isModoApertura(iterator->modo, MYO_RDWR))
			{
				pthread_mutex_unlock(mutex);
				logM("iterator->filename = %s\n", iterator->fileName);
				return 1;
			}
		}
		iterator = iterator->next;
	}
	pthread_mutex_unlock(mutex);
	
	logM("[fileAlreadyOpenedInWrite] - Nessun file aperto con questo nome in scrittura: '%s' \n",filename);

	return 0;
}


/**
 * @brief Dice se modo_client contiene il modo "modo".
 * 
  */
int isModoApertura(int modo_client, int modo)
{
	if((modo_client&modo)==modo)
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * @brief Rimuove il nodo openedfile associato al ptid.
 * @todo da fare.
 */
void closeOpenedFile(unsigned long int ptid)
{
	pthread_mutex_lock(mutex);
	
	pthread_mutex_unlock(mutex);
}
/**
 * Libera lo spazio occupato dal puntatore
 * @todo da finire.
 */
void freeOpenedFile(OpenedFile* id)
{
	free(id->fileName);
	fclose(id->fp);
	memset(id,0,sizeof(OpenedFile));
}


/**
 * @brief rimuove collegamenti tra client e file aperti nella sessione
 * NOTA: ACCESSO SINCRONIZZATO CON MUTEX
 * @param int sd socket descriptor del client
 */
void closeClientSession(unsigned long int ptid) 
{
	pthread_mutex_lock(mutex);	
	OpenedFile* iterator = *openedFileLinkedList;
	// Dal main di server.c richiamo questa funzione quando si interrompe la socket
	// Quindi iterator potrebbe essere null.
	if(iterator == NULL)
	{
		pthread_mutex_unlock(mutex);		
		return;
	}
	OpenedFile* preIterator = NULL;
	while(TRUE)
	{
		if(iterator->ptid == ptid)
		{
			
			//OpenedFile* temp = iterator;

			
			if(preIterator == NULL) //primo della lista
			{
				logM("[closeClientSession] - individuato file primo: '%s'\n", iterator->fileName);
				*openedFileLinkedList = iterator->next;
				freeOpenedFile(iterator);				
				iterator->next = *free_head;
				*free_head = iterator;
				
			}
			else
			{
				if(iterator->next == NULL) //ultimo della lista
				{
					logM("[closeClientSession] - individuato file ultimo: %s - pronto alla chiusura\n", iterator->fileName);
					iterator->next = *free_head;
					*free_head = iterator;
					preIterator->next = NULL;
				}
				else //ne primo ne ultimo della lista
				{
					logM("[closeClientSession] - individuato file medio: %s - pronto alla chiusura\n", iterator->fileName);
					preIterator->next = iterator->next;
					freeOpenedFile(iterator);
					iterator->next = *free_head;
					*free_head = iterator;
					
				}	
			}
			break;
		}
		else
		{

			if(iterator->next != NULL) //se ci sono ancora file da controllare
			{
				preIterator = iterator;
				iterator = iterator->next;
			}
			else
			{
				break;
			}
		}
	}
	pthread_mutex_unlock(mutex);
	logM("[CloseClientSession] Numero connessioni rimaste: %d\n", countOpenedFile());
	logM("[closeClientSession] - Connessione chiusa.\n");
}

int getTransferSocket()
{
	pthread_mutex_lock(mutex);
	OpenedFile* iterator = *openedFileLinkedList;
	while(iterator != NULL)
	{
		if(iterator->ptid == getptid())
		{
			pthread_mutex_unlock(mutex);
			return iterator->transferSockId;
		}
		iterator = iterator->next;
	}
	pthread_mutex_unlock(mutex);
	return 0;
}
/**
 * @brief Ritorna il nome del file aperto da questo ptid.
 * NOTA: ACCESSO SINCRONIZZATO CON MUTEX
 */
char* getFileName()
{
	pthread_mutex_lock(mutex);	
	OpenedFile* iterator = *openedFileLinkedList;
	while(iterator != NULL)
	{
		if(iterator->ptid == getptid())
		{
			pthread_mutex_unlock(mutex);
			return iterator->fileName;
		}
		iterator = iterator->next;
	}
	pthread_mutex_unlock(mutex);
	return NULL;
}

/**
 * @brief Ritorna l' opened file associato a questo ptid
 */
OpenedFile* getOpenedFile()
{
	pthread_mutex_lock(mutex);

	OpenedFile* iterator = *openedFileLinkedList;
	while(iterator != NULL)
	{
		if(iterator->ptid == getptid())
		{
			pthread_mutex_unlock(mutex);
			return iterator;
		}
		iterator = iterator->next;
	}
	
	pthread_mutex_unlock(mutex);
	return NULL;
}

int countOpenedFile()
{
	int i = 0;
	pthread_mutex_lock(mutex);
	OpenedFile* iterator = *openedFileLinkedList;
	while(iterator != NULL)
	{
		i++;
		iterator = iterator->next;
	}
	pthread_mutex_unlock(mutex);
	return i;
}
