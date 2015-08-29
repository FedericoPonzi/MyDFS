#include <stdio.h>
#include "inc/Utils.h"
#include "inc/Config.h"
#include "inc/CLOSE.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <errno.h>

int getNumberOfChanges(MyDFSId* id);

/**
 * @param dfsid: L' id del file che l' utente vuole chiudere.
 * @return -1 in caso di errore, 0 altrimenti.
 * Il comando CLOSE invia anche il numero di write che sono state eseguite, quindi quante send il client dovra' mandare al server.
 * 
 */
int mydfs_close(MyDFSId* id)
{
		logM("[Close] '%s'\n", id->filename);

		char closeCommand[strlen(CLOSECOMMAND)+getNumberLenght(getNumberOfChanges(id))+2];
		
		sprintf(closeCommand, "%s %d\n", CLOSECOMMAND, getNumberOfChanges(id));
		
		if(send(id->socketId, closeCommand, strlen(closeCommand), 0) < 0)
		{
			perror("1-send");
			return -1;
		}
		
		//Cancello tutto quanto:
		
		close(id->socketId);
		close(id->transferSockId);
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
		
		ReadOp* iteratorr = id-> readList;
		ReadOp* tempr;
		while(iteratorr != NULL)
		{
			tempr = iteratorr->next;
			free(iteratorr);
			iteratorr = tempr;
		}
		iteratorr = NULL;
		free(id);
		
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
		char buffcommand[5+getNumberLenght(iterator->size)+2]; // "SIZE %d", iterator->size;
		sprintf(buffcommand, "SIZE %d\n", iterator->size);
		
		if(send(id->socketId, buffcommand, strlen(buffcommand), 0) < 0)
		{
			perror("1-send");
			return -1;
		}
		
		void* buffer = malloc(iterator->size);
		
		fseek(id->fp, iterator->pos, SEEK_SET);
		
		fread(buffer, 1, iterator->size, id->fp);
		
		if(send(id->socketId, buffer, sizeof(buffer), 0) <0)
		{
			perror("Send error sending write.");
			return -1;
		}
		
		free(buffer);
		iterator = iterator->next;		
	}
	return 0;
}

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
