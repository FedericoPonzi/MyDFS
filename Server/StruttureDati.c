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
#include "inc/Utils.h"
#include "inc/StruttureDati.h"
#include <sys/types.h>
#include <sys/socket.h>

/**
 * @brief Aggiunge l' oggetto in input alla fine della lista linkata dei file aperti OpenedFile
 * 
 * @todo Dovrebbe tenere da conto anche il modo di apertura
 * Se non esiste la radice mainFile la crea. Altrimenti scorre tutti i nodi fino ad arrivare all' elemento in coda, crea un nuovo
 * nodo con i dati in input e lo appende alla fine.
 */
void appendOpenedFile(char* nomefile, int modo, int socket)
{

	if(fileAlreadyOpen(nomefile, modo, socket) == -1)
	{
		logM("Ha provato ad aprire un file gia' aperto: '%s' \n", nomefile);
		char answer[100] = "File gia' aperto n";
		send(socket, answer, strlen(answer), 0);
	}
	else if(fileAlreadyOpen(nomefile, modo, socket) == 0)
	{
		OpenedFile* prova;
		prova = (OpenedFile*) malloc(sizeof(OpenedFile));
		prova->fileName = nomefile;
		
		prova->socketIdList = (SocketIdList*) malloc(sizeof(SocketIdList));
		prova->socketIdList->socketId = socket;
		prova->socketIdList->next = NULL;
		prova->socketIdList->modo = modo;
		prova->next = NULL;
		if(openedFileLinkedList == NULL)
		{
			openedFileLinkedList = prova;
			logM("[OpenFile] Main non esiste. Lo creo. '%s'\n", openedFileLinkedList->fileName);
		}
		else
		{
			logM("[OpenFile] Main esiste '%s' \n", openedFileLinkedList->fileName);
			OpenedFile* iterator = openedFileLinkedList;
			while(iterator->next != NULL)
			{
				iterator = iterator->next;
			}
			
			iterator->next = prova;
		}
		
		logM("[OpenFile] Aggiunto. Nome: '%s' \n", prova->fileName);
	   }
	else
	{
		OpenedFile* iterator = openedFileLinkedList;
		do
		{
			if(strcmp(iterator->fileName, nomefile)==0)
			{
				aggiungiSocketId(iterator->socketIdList, socket, modo);
			}
			if(iterator->next != NULL)
			{
				iterator = iterator->next;
			}
		}while(iterator->next != NULL);
	}
	   
	
}

/**
 * @brief Controlla se un file con quel nome e' gia' stato aperto e se è stato aperto dal client che ne sta richiedendo nuovo accesso.
 *
 *  
   @param filename Il nome del file da controllare
 * @param socketId l'id della socket del client
 * @param modo_client il modo con cui il client intende accedere al file
 * 
 * @return modo se il file è stato già aperto da client che sta richiedendo nuovo accesso,
 * @return 1 se il file e' gia' aperto da altro client in mod. bloccante
 * @return 0 se file non è ancora stato aperto
 */

int fileAlreadyOpen(char* filename, int modo_client, int socketId)
{
	if(openedFileLinkedList == NULL) return FALSE;
	
	OpenedFile* iterator = openedFileLinkedList;
	
	do
	{
		if(strcmp(iterator->fileName, filename) == 0)
		{
			if(socketIdAlreadyAdded(iterator->socketIdList, socketId))
			{
				return getModoFromSocketId(iterator->socketIdList, socketId);
			}
			if(isModoApertura(getModoFromSocketId(iterator->socketIdList, socketId), MYO_EXLOCK) || isModoApertura(getModoFromSocketId(iterator->socketIdList, socketId), MYO_WRONLY) || isModoApertura(getModoFromSocketId(iterator->socketIdList, socketId), MYO_RDWR))
			{
				return -1;
			}
			return 0;
		}
		iterator = iterator->next;
	}while(iterator != NULL);
	
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
@brief aggiunge elemento a lista linkata di socket o aggiunge modo a socket già aggiunta
* @param SocketIdList* sl puntatore alla prima socket della lista
* @param int socketId id della socket da aggiungere
* @param modo modo di apertura file da parte di nuova socket
* @return void
* */
void aggiungiSocketId(SocketIdList* sl, int socketId, int modo)
{
	SocketIdList* iterator = sl;
	while(iterator->next != NULL || iterator->socketId != socketId)
	{
		iterator = iterator->next;
	}
	
	if(iterator->socketId == socketId)
	{
		iterator->modo = iterator->modo | modo;
	}
	else
	{		
		SocketIdList* newSI = malloc(sizeof(SocketIdList));
		newSI->socketId = socketId;
		newSI->modo = modo;
		iterator->next = newSI;
	}
}

/**
@brief controllo presenza socketId in lista socket
* @param SocketIdList* sl puntatore alla prima socket della lista
* @param int socketId id della socket da controllare
* @return TRUE (1) se id gia' presente in lista, FALSE (0) altrimenti
* */
int socketIdAlreadyAdded(SocketIdList* sl, int socketId)
{
	SocketIdList* iterator = sl;
	do
	{
		if(iterator->socketId == socketId)
		{
			return TRUE;
		}
		iterator = iterator->next;
	}while(iterator->next != NULL);
	return FALSE;
}

/**
 * @brief ottieni modo (anche multiplo) di accesso di un certo socketId su un certo file
 * @param SocketIdList* sl puntatore alla prima socket della lista
 * @param int socketId id della socket di cui ottenere modo
 * @return modo se socket già ha aperto file, -1 altrimenti
 */
int getModoFromSocketId(SocketIdList* sl, int socketId)
{
	SocketIdList* iterator = sl;
	do
	{
		if(iterator->socketId == socketId)
		{
			return iterator->modo;
		} 
		iterator = iterator->next;
	}while(iterator->next != NULL);
	return -1;
}

/**
 * @brief rimuove collegamenti tra client e file aperti nella sessione
 * @param int sd socket descriptor del client
 */
void closeClientSession(int sd)
{
	OpenedFile* iterator = openedFileLinkedList;
	SocketIdList* iterator2;
	SocketIdList* preIterator2;
	while(TRUE)
	{
		iterator2 = iterator->socketIdList;
		do
		{
			/*stampe di debug:
			logM("nuovo iterator: %s\n", iterator->fileName);
			logM("iterator->socketId = %d\n", iterator2->socketId);
			logM("socket_closing = %d\n", sd);*/
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
}
