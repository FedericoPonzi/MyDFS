#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
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

/**
 * @brief Gestisce il comando Close.
 * @param int socket socket del client che termina sessione
 */
void handleCloseCommand(char* command, int socket)
{	
	OpenedFile* id = getOpenedFile();
	
	handleWrites(getNumberOfChanges(command), id);
	
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
		}
		printf("Messaggio: %d dati,  '%s'\n", n,  messaggio);
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
			char* buffer = malloc(buffSize); /**@todo : cambiare a void*/
			n = recv(id->transferSockId, buffer, size > FILESIZE ? FILESIZE : size, 0);
			printf("Messaggio size: '%d', '%s'\n", n, buffer);
			
			if(n < 0)
			{
				perror("Recv: ");
				return -1;
			}
			
			n = fwrite(buffer, 1, n, fp);
			printf("Scritti %d dati nel file. \n", n);
			
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
