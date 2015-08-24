#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "inc/Config.h"
#include "inc/Utils.h"
#include "inc/READ.h"
#include "inc/Cache.h"

int sendReadCommand(MyDFSId* id, int pos);
int readFrom(MyDFSId* id, int sizeRimasta, int pos );
int byteMancanti(FILE* fp);
int appendReadRequest(MyDFSId* id, int pos, int size);
/**
 * name: mydfs_read
 * 
 * @param id: L' id del file da cui vuole leggere.
 * @param pos: Posizione da cui vuole leggere
 * @param ptr: puntatore al buffer in cui andra a legere
 * @param size: quanto vuole leggere
 * @return -1 in caso di errore, n bytes letti altrimenti.
 * 
 * @todo quando ricevo i 65000 bytes di dati, devo stare attento a non sovrascrivere nel file dei dati che ho scritto (e che quindi non dovrei sovrascrivere con una read).
 */
 
int mydfs_read(MyDFSId* id, int pos, void *ptr, unsigned int size)
{
	logM("[Read] Filename '%s' \n", id->filename);	

	CacheRequest req;
	int primoBuco;
	int daRicevere;
	//Mi trovo il primo buco nel file di cache:
	while((primoBuco = readRequest(id, pos, size, &req)))
	{
		logM("mydfs_read: primo buco: %d\n", primoBuco);
		
		//Mando la richiesta di read da una certa posizione:
		daRicevere = sendReadCommand(id, primoBuco);
		
		//Adesso nella socket ho pronti daRicevere bytes per la lettura. Me li leggo e li scrivo nel file dalla posizione
		// primoBuco
		if(readFrom(id, daRicevere, primoBuco))
		{
			logM("readFrom:Erroreee \n");
		}
		
		appendReadRequest(id, primoBuco, daRicevere); 
	}
		
	//Sposto il puntatore:
	fseek(id->fp, 0, pos);
	
	//Adesso nel file ho tutto il necessario, quindi eseguo una normale read:
	int n = fread(ptr, 1, size, id->fp);

	if(n==0)
	{
		if(ferror(id->fp))
		{
			logM("Error nella lettura\n\n");
		}
		else if(feof(id->fp))
		{
			logM("File terminato");
		}
	}
	return n;
}


/**
 * @brief Manda il comando di READ, e aspetta come risposta la dimensione di quanto deve leggere all'interno della risposta.
 */
int sendReadCommand(MyDFSId* id, int pos)
{
	//Mando la read
	char readPosition[100];
	sprintf(readPosition, "%d", pos);
	
	char readCommand[strlen(READCOMMAND) + strlen(readPosition)+1]; //es: "READ 1", con 1 = la posizione
	sprintf(readCommand, "%s %s\n", READCOMMAND, readPosition);
	logM("Mando richiesta di READ:'%s'", readCommand);
	send(id->socketId, readCommand, strlen(readCommand), 0);
	
	//Ricevo la dimensione della parte letta
	char fileSize[100];
	int nRecv = recv(id->socketId, fileSize, sizeof(fileSize)-1, 0);
	fileSize[nRecv] = '\0';
	logM("Messaggio ricevuto: '%s'\n", fileSize);
	
	//Metto dentro a fileSize la quantita' di byte che mi aspetto di trovare nella socket:
	char substringa[strlen(fileSize-5)];
	strcpy(substringa, &fileSize[5]);
	strcpy(fileSize, substringa);
	
	int toRet = strtol(fileSize, NULL, 10);
	logM("DimensioneLetta: '%d'\n", toRet);
	return toRet;
}

/**
 * @brief Legge dalla socket di trasferimento buffsize dati.
 * @return 1 se c'e' un errore con la recv
 * il risultato di writeCache altrimenti.
 * 
 * @see writeCache
 */
 
int readFrom(MyDFSId* id, int sizeRimasta,  int pos )
{
	char buff[sizeRimasta];
	logM("Pronto per la read!\n");
	
	int nRecv = recv(id->socketId, buff, sizeRimasta, 0);
	if(nRecv < 0)
	{
		return 1;
	}
	else
	{
		//Scrivo il contenuto nella cache:
		return writeCache(id, buff, nRecv, pos);
	}
	
}


/**
 * @brief Torna il numero di byte prima della fine del file puntato da fp
 */
int byteMancanti(FILE* fp)
{
	int posAttuale, dimen;
	posAttuale = ftell(fp);
	fseek(fp,0,SEEK_END);
	dimen = ftell(fp);
	fseek(fp,posAttuale,SEEK_SET);
	return dimen-posAttuale;
}

/**
 * @brief Aggiunge un nodo alla lista di reads
 * @return 0 Se tutto e' andato bene
 * @return 1 errore
 */
int appendReadRequest(MyDFSId* id, int pos, int size)
{
	ReadOp* readOp = malloc(sizeof(ReadOp));
	readOp->pos = pos;
	readOp->size = size;
	
	if(readOp == NULL)
	{
		perror("appendReadRequest: malloc ");
		return 1;
	}		
	ReadOp* iterator = id->readList;
	while(iterator != NULL && iterator->next != NULL)
	{
		iterator = iterator->next;
	}
	if(!iterator)
	{
		id->readList = readOp;
	}
	else
	{
		iterator->next = readOp;
	}
	return 0;	
}
