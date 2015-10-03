#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
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
 * @param pos: Posizione da cui vuole leggere (una delle seek).
 * @param ptr: puntatore al buffer in cui andra a legere
 * @param size: quanto vuole leggere
 * @return -1 in caso di errore, n bytes letti altrimenti.
 * 
 * WinReady!
 */
 
int mydfs_read(MyDFSId* id, int pos, void *ptr, unsigned int size)
{

	
	CacheRequest req;

	int daRicevere;
	
	//Sposto il puntatore:
    if(pos == MYSEEK_END)
    {
        logM("Myseekend\n");
        fseek(id->fp, id->filesize, SEEK_SET);
    }
    else
    {
        fseek(id->fp, 0, pos);
	}
    
	//Posizione del puntatore
	int posizione = ftell(id->fp);
    
	/*
	 * Piccolo hack: per qualche motivo, size da dei problemi con la read.
	 * Per ora mi leggo solo quello che mi serve fino alla fine del file.
	 */
	 size = posizione+size > id->filesize ? id->filesize-posizione : size;
    logM("size: %d, filesize:%d \n", size, id->filesize);
	logM("Posizione del puntatore all' inizio della read:%d\n", posizione);
	//Mi trovo il primo buco nel file di cache:
	while(readRequest(id, posizione, size, &req))
	{
		logM("mydfs_read: primo buco: %d\n", req.pos);
		
		//Mando la richiesta di read da una certa posizione:
		daRicevere = sendReadCommand(id, req.pos);
		
		//Adesso nella socket ho pronti daRicevere bytes per la lettura. Me li leggo e li scrivo nel file dalla posizione
		// primoBuco
		if(readFrom(id, daRicevere, req.pos))
		{
			logM("readFrom:Erroreee \n");
		}
		
		appendReadRequest(id, req.pos, daRicevere); 
	}
		
	
	//Adesso nel file ho tutto il necessario, quindi eseguo una normale read:
	int n = fread(ptr, 1, size, id->fp);

	if(n==0)
	{
		if(ferror(id->fp))
		{
			logM("Error nella lettura\n");
		}
		else if(feof(id->fp))
		{
			logM("File terminato\n");
		}
	}
	logM("Posizione del puntatore ala fine della read:%d\n", ftell(id->fp));

	return n;
}


/**
 * @brief Manda il comando di READ, e aspetta come risposta la dimensione di quanto deve leggere all'interno della risposta.
 * WinReady
 */
int sendReadCommand(MyDFSId* id, int pos)
{
	//Mando la read

	char readCommand[strlen(READCOMMAND) + 	getNumberLenght(pos)+1]; //es: "READ 1", con 1 = la posizione
	sprintf(readCommand, "%s %d\n", READCOMMAND, pos);
	logM("Mando richiesta di READ:'%s'\n", readCommand);
	send(id->socketId, readCommand, strlen(readCommand), 0);
	
	//Ricevo la dimensione della parte letta
	char fileSize[100];
	int nRecv = recv(id->socketId, fileSize, sizeof(fileSize)-1, 0);
	if(nRecv < 0)
	{
		perror("Read command recv:");
	}

	//Metto dentro a fileSize la quantita' di byte che mi aspetto di trovare nella socket:
	char substringa[strlen(fileSize+5)];
	strcpy(substringa, &fileSize[5]);
	strcpy(fileSize, substringa);
	int toRet = strtol(fileSize, NULL, 10);
    logM("Mi sta mandando: %d dati.", toRet);
	return toRet;
}

/**
 * @brief Legge dalla socket di trasferimento buffsize dati.
 * @return 1 se c'e' un errore con la recv
 * il risultato di writeCache altrimenti.
 * 
 * @see writeCache
 * @todo La recv dovrebbe continuare finche' non ho tutto sizeRiamsta.
 * Winready
 */
 
int readFrom(MyDFSId* id, int sizeRimasta,  int pos )
{
	char temp[sizeRimasta];
	char buff[sizeRimasta];
	int i = 0;
	int nRecv = 0;
	/*
	 * Potrei ricevere meno di sizeRimasta la prima volta. Finche' non ricevo tutti i dati continuo la recv.
	 */
	while( i < sizeRimasta)  
	{
		nRecv = recv(id->socketId, temp, sizeRimasta, 0);
		memcpy(buff+i, temp, nRecv);
		i+=sizeRimasta;
	}
	if(nRecv < 0)
	{
		return 1;
	}
	else
	{
		//Scrivo il contenuto nella cache:
		return writeCache(id, buff, i, pos);
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
	WaitForSingleObject(id->readListMutex, INFINITE);

	ReadOp* readOp = malloc(sizeof(ReadOp));
	readOp->pos = pos;
	readOp->size = size;
	readOp->next = NULL;
	
	if(readOp == NULL)
	{
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
	ReleaseMutex(id->readListMutex);

	return 0;	
}
