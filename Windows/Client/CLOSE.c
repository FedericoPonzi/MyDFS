/**
 * WinReady!
 */
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include "inc/Utils.h"
#include "inc/Config.h"
#include "inc/CLOSE.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>   
#include <winsock2.h>
#include <ws2tcpip.h>
#include <errno.h>

int getNumberOfChanges(MyDFSId* id);
int uploadChanges(MyDFSId* id);

/**
 * @param dfsid: L' id del file che l' utente vuole chiudere.
 * @return -1 in caso di errore, 0 altrimenti.
 * Il comando CLOSE invia anche il numero di write che sono state eseguite, quindi quante send il client dovra' mandare al server.
 * 
 */
int mydfs_close(MyDFSId* id)
{
		logM("[Close] '%s'\n", id->filename);

		char closeCommand[100];
		
		sprintf(closeCommand, "%s %d\n", CLOSECOMMAND, getNumberOfChanges(id));
		
		if(send(id->socketId, closeCommand, strlen(closeCommand), 0) < 0)
		{
			perror("1-send");
			return -1;
		}
		
		uploadChanges(id);
		//Cancello tutto quanto:
        //shutdown(id->socketId, SD_SEND);
		//closesocket(id->transferSockId);
		free(id->indirizzo);
		unlink(id->filename);
		free(id->filename);
		
		WriteOp* iteratorw = id->writeList;
		WriteOp* temp;
		while(iteratorw != NULL)
		{
			temp = iteratorw->next;
			free(iteratorw);
			iteratorw = temp;
		}
		iteratorw = NULL;
		
		WaitForSingleObject(id->readListMutex, INFINITE);
		ReadOp* iteratorr = id-> readList;
		ReadOp* tempr;
		while(iteratorr != NULL)
		{
			tempr = iteratorr->next;
			free(iteratorr);
			iteratorr = tempr;
		}
		iteratorr = NULL;
		ReleaseMutex(id->readListMutex);
		CloseHandle(id->readListMutex);
		free(id);
		logM("Chiusura effettuata con successo.");
		
		return 0;
}

/**
 * @brief Carica i cambiamenti fatti dal client con le write sul server
 */
int uploadChanges(MyDFSId* id)
{
	WriteOp* iterator = id->writeList;

	while(iterator != NULL)
	{		
		char buffcommand[100]; // "POS %d SIZE %d", iterator->size;
		memset(buffcommand, 0, 100);
		sprintf(buffcommand, "POS %d SIZE %d\n", iterator->pos, iterator->size);
		logM("Sto inviando il comando: %s\n", buffcommand);
		if(send(id->transferSockId, buffcommand, sizeof(buffcommand), 0) < 0)
		{
			perror("2-send");
			return -1;
		}
		logM("Fatta la send! dovrei invaire %d dati in teoria. \n", iterator->size);
		
		/** @todo spezzare in piu read.*/
		void* buffer = malloc(iterator->size);
		
		fseek(id->fp, iterator->pos, SEEK_SET);
		fread(buffer, 1, iterator->size, id->fp);
		
		logM("Sto inviando %d dati.\n", iterator->size);
		if(send(id->transferSockId, buffer, iterator->size, 0) <0)
		{
			perror("Send error sending write.");
			return -1;
		}
        //@todo: forse bisognerebbe aspettare un messaggio di rs
		logM("Inviati.");
		free(buffer);
		iterator = iterator->next;		
	}

	return 0;
}

/**
 * Ritorna il numero di write effettuate dal client
 */
int getNumberOfChanges(MyDFSId* id)
{
	int toRet = 0;
	WriteOp* iterator = id->writeList;
	while(iterator != NULL)
	{
		iterator = iterator->next;
		toRet++;
	}
	return toRet;
}
