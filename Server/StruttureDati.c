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
#include "inc/Config.h"
#include "inc/Utils.h"
#include "inc/StruttureDati.h"


OpenedFile ** openedFileLinkedList = NULL;
OpenedFile ** free_head;
static pthread_mutex_t *mutex;



/**
 * @brief Alloca la memoria necessaria per gestire le strutture dati.
 * @todo Dovrebbe trovarsi dentro al Config.c
 */
void allocaEInizializzaMemoria()
{
	void *ptr;
    size_t region_sz = 0;

    /* Space for the nodes */
    region_sz += sizeof(OpenedFile)*numeroCon;
	
	/* Space for house-keeping pointers */
    region_sz += sizeof(openedFileLinkedList)+sizeof(free_head);

	/* Space for the mutex */
    region_sz += sizeof(*mutex);
	
	/* Spazio per il mutex dell'accept e dell' int numbero figli vivi */
	region_sz += sizeof(pthread_mutex_t);
	region_sz += sizeof(int);
	
	logM("[Initialize Memory] Sto per allocare %lu spazio.\n", region_sz);
    ptr = mmap(NULL, region_sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap(2) failed");
        exit(EXIT_FAILURE);
    }

    /* Set up everything */
    
    /**Indirizzo di serverconnectionsstruct */
    acceptMutex = ptr+region_sz-sizeof(pthread_mutex_t)-sizeof(int);
    
    numberAliveChilds = ptr+region_sz-sizeof(int);
    
    mutex = ptr;
    
    free_head = (OpenedFile **) (((char *) ptr)+sizeof(*mutex));
   
    openedFileLinkedList = free_head+1;

    *free_head = (OpenedFile *) (openedFileLinkedList+1);

    *openedFileLinkedList = NULL;

    /* Initialize free list */
    int i;
    OpenedFile *curr;

    for (i = 0, curr = *free_head; i < numeroCon-1; i++, curr++) {
        curr->next = curr+1;
	}

    curr->next = NULL;

    pthread_mutexattr_t mutex_attr;
    if (pthread_mutexattr_init(&mutex_attr) < 0) {
        perror("Failed to initialize mutex attributes");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) < 0) {
        perror("Failed to change mutex attributes");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(mutex, &mutex_attr) < 0) {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }
}


/**
 * @brief Aggiunge l' oggetto in input alla fine della lista linkata dei file aperti OpenedFile
 * 
 * Se non esiste la radice mainFile la crea. Altrimenti scorre tutti i nodi fino ad arrivare all' elemento in coda, crea un nuovo
 * nodo con i dati in input e lo appende alla fine.
 */
int appendOpenedFile(char* nomeFile, int modo, int socket)
{
	logM("Appendo il file aperto.\n");
	//Se richiede una scrittura, e il file e' già aperto in scrittura:
	if(fileAlreadyOpenedInWrite(nomeFile, modo, socket)) //Aperto in scrittura
	{
		logM("Non posso farlo john\n");
		return 0;
	}
	logM("Prendo il lock: \n");
	pthread_mutex_lock(mutex);

	OpenedFile *n = *free_head;

	if (n == NULL) {
		pthread_mutex_unlock(mutex);
		assert(0);
	}

	*free_head = (*free_head)->next;
	n->fileName = nomeFile;
	n->socketId = socket;
	n->modo = modo;
	n->next = *openedFileLinkedList;
	*openedFileLinkedList = n;

	pthread_mutex_unlock(mutex);
	logM("Lock rilasciato. \n");

	return 1;
}

/**
 * @brief Controlla se un file con quel nome e' gia' stato aperto in una modalita' che non permette l'accesso da parte di altri client
 *
 */

int fileAlreadyOpenedInWrite(char* filename, int modo_client, int socketId)
{
	logM("File already opened in write? \n");

	if(openedFileLinkedList == NULL) return FALSE;
	
	OpenedFile* iterator = *openedFileLinkedList;
	while(iterator != NULL)
	{
		if(strcmp(iterator->fileName, filename) == 0)
		{
			if(isModoApertura(iterator->modo, MYO_EXLOCK) || isModoApertura(iterator->modo, MYO_WRONLY) || isModoApertura(iterator->modo, MYO_RDWR))
			{
				return -1;
			}
			return FALSE;
		}
		iterator = iterator->next;
	}
	
	logM("Nessun file aperto con questo nome: '%s' \n",filename);

	return 0;
}


/**
 * @brief Dice se modo_client contiene il modo modo.
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
void closeClientSession(int sd, char* fileName) //SISTEMA LOOP
{
	OpenedFile* iterator = *openedFileLinkedList;
	OpenedFile* preIterator = NULL;
	while(TRUE)
	{
		if(iterator->socketId == sd && (strcmp(iterator->fileName, fileName) == 0))
		{
			
			//OpenedFile* temp = iterator;
			pthread_mutex_lock(mutex);
			
			if(preIterator == NULL) //primo della lista
			{
				logM("individuato file primo: %s - pronto alla chiusura\n", iterator->fileName);
				*openedFileLinkedList = iterator->next;
				
				iterator->next = *free_head;
				*free_head = iterator;
				
				//iterator = temp->next;
			}
			else
			{
				if(iterator->next == NULL) //ultimo della lista
				{
					logM("individuato file ultimo: %s - pronto alla chiusura\n", iterator->fileName);
					iterator->next = *free_head;
					*free_head = iterator;
					preIterator->next = NULL;
				}
				else //nè primo nè ultimo della lista
				{
					logM("individuato file medio: %s - pronto alla chiusura\n", iterator->fileName);
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
			logM("skip\n");
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
	
	/*
	OpenedFile* iterator = openedFileLinkedList;
	SocketIdList* iterator2;
	SocketIdList* preIterator2;
	while(TRUE)
	{
		iterator2 = iterator->socketIdList;
		do
		{
			/stampe di debug:
			logM("nuovo iterator: %s\n", iterator->fileName);
			logM("iterator->socketId = %d\n", iterator2->socketId);
			logM("socket_closing = %d\n", sd);
			if(iterator2->socketId == sd) //se iterator2->SocketId è uguale a quella in input
			{
				logM("chiudo %s\n", iterator->fileName);
				if(preIterator2 != NULL) //se iterator2 non è primo della lista
				{
					if(iterator2->next != NULL) //se iterator2 non è ultimo della lista
					{
							preIterator2->next = iterator2->next;
							SocketIdList temp_it = *iterator2;
							free(iterator2);
							iterator2 = temp_it.next;
							preIterator2 = preIterator2->next;
					}
					else //se iterator2 è ultimo della lista
					{
							free(iterator2);
							preIterator2->next = NULL;
					}
				}
				else //se iterator2 è primo della lista
				{
					if(iterator2->next != NULL) //se iterator2 non è ultimo della lista
					{
							SocketIdList temp_it = *iterator2;
							free(iterator2);
							iterator->socketIdList=temp_it.next;
					}
					else //se iterator2 è ultimo della lista
					{
							free(iterator2);
							iterator->socketIdList=NULL;
					}
				}
				break;	
			}
			else //se iterator2->SocketId è diversa da quella di input
			{
				if(iterator2->next != NULL)
				{	
					iterator2 = iterator2->next;
					preIterator2 = iterator2;
				}
			}
		}while(iterator2->next != NULL);
		
		if(iterator->next != NULL) //procedo finchè ho esaminato tutti i file aperti
		{
			iterator = iterator->next;
		}
		else
		{
			break;
		}
		
	}
	*/
}
