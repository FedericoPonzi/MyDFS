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


OpenedFile ** openedFileLinkedList = NULL;
OpenedFile ** free_head;
pthread_mutex_t *mutex;



/**
 * @brief Aggiunge l' oggetto in input alla fine della lista linkata dei file aperti OpenedFile
 * 
 * Se non esiste la radice mainFile la crea. Altrimenti scorre tutti i nodi fino ad arrivare all' elemento in coda, crea un nuovo
 * nodo con i dati in input e lo appende alla fine.
 */
int appendOpenedFile(char* nomeFile, int modo)
{
	logM("[appendOpenedFile] - Appendo il file aperto.\n");
	//Se richiede una scrittura, e il file e' già aperto in scrittura:
	if(fileAlreadyOpenedInWrite(nomeFile)) //Aperto in scrittura
	{
		return 1;
	}

	if(checkModoOpen(nomeFile, modo))
	{
		logM("Checkmodoopen error\n");
		return -3;
	}

	pthread_mutex_lock(mutex);

	OpenedFile *n = *free_head;
	
	if (n == NULL) 
	{
		pthread_mutex_unlock(mutex);
		assert(0);
	}

	*free_head = (*free_head)->next;
	n->fileName = nomeFile;
	n->ptid = getptid();
	n->modo = modo;
	n->next = *openedFileLinkedList;
	*openedFileLinkedList = n;

	pthread_mutex_unlock(mutex);

	return 0;
}

/**
 * @brief controlla se la open è realizzabile con il modo richiesti
 */
int checkModoOpen(char *nomeFile, int modo)
{
	OpenedFile* iterator = *openedFileLinkedList;
	
	//copio in temp_path il rootPath
	char temp_path[40];
	strcpy(temp_path, rootPath);
	strcat(temp_path, nomeFile);
	
	while(iterator != NULL)
	{
		if(strcmp(iterator->fileName, nomeFile) == 0)
		{
			if(isModoApertura(modo, MYO_RDONLY) || isModoApertura(modo, MYO_WRONLY) || isModoApertura(modo, MYO_RDWR) || 
			  (isModoApertura(modo, MYO_TRUNC) && (isModoApertura(modo, MYO_WRONLY) || isModoApertura(modo, MYO_RDWR))) || isModoApertura(modo, MYO_EXCL))
			{
				if(access(temp_path, F_OK) != -1)
				{
					return 0;
				}
				else
				{
					return -3;
				}
			}
		}
		iterator = iterator->next;
	}
	
	
	return 0;
}

/**
 * @brief Controlla se un file con quel nome e' gia' stato aperto in una modalita' che non permette l'accesso da parte di altri client
 *
 */

int fileAlreadyOpenedInWrite(char* filename)
{

	if(openedFileLinkedList == NULL) return FALSE;
	
	OpenedFile* iterator = *openedFileLinkedList;
	while(iterator != NULL)
	{
		if(strcmp(iterator->fileName, filename) == 0)
		{
			if(isModoApertura(iterator->modo, MYO_EXLOCK) || isModoApertura(iterator->modo, MYO_WRONLY) || isModoApertura(iterator->modo, MYO_RDWR))
			{
				return 1;
			}
		}
		iterator = iterator->next;
	}
	
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
 * @brief rimuove collegamenti tra client e file aperti nella sessione
 * @param int sd socket descriptor del client
 */
void closeClientSession(int ptid) //fileName va rimosso, la session è relativa ad un solo file
{
	OpenedFile* iterator = *openedFileLinkedList;
	OpenedFile* preIterator = NULL;
	while(TRUE)
	{
		if(iterator->ptid == ptid)
		{
			
			//OpenedFile* temp = iterator;
			pthread_mutex_lock(mutex);
			
			if(preIterator == NULL) //primo della lista
			{
				logM("[closeClientSession] - individuato file primo: %s - pronto alla chiusura\n", iterator->fileName);
				*openedFileLinkedList = iterator->next;
				
				iterator->next = *free_head;
				*free_head = iterator;
				
				//iterator = temp->next;
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
					
					iterator->next = *free_head;
					*free_head = iterator;
					
					//iterator = temp->next;
				}	
			}
			pthread_mutex_unlock(mutex);
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
	logM("[closeClientSession] - Connessione chiusa.\n");
}
