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
 * @todo per qualche motivo, la fread ritorna 0 O_o pero' tipo la copia in un altro file sembra funzionare.
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
	 size = posizione+size >= id->filesize ? id->filesize-posizione : size;
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
    else if(nRecv == 0)
    {
        perror("recv: il peer e' andato.");
    }

	//Metto dentro a fileSize la quantita' di byte che mi aspetto di trovare nella socket:
    fileSize[nRecv] = '\0';
	char substringa[strlen(fileSize+5)];
	strcpy(substringa, &fileSize[5]);
	strcpy(fileSize, substringa);

    int toRet = strtol(fileSize, NULL, 10);
    printf("filesize: %s\n", fileSize);
    logM("Mi sta mandando: %d dati.\n", toRet);
	return toRet;
}

/**
 * @brief Legge dalla socket di trasferimento buffsize dati.
 * @return 1 se c'e' un errore con la recv
 * il risultato di writeCache altrimenti.
 * 
 * @see writeCache
 * @todo La recv dovrebbe continuare finche' non ho tutto sizeRiamsta.
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
        if(nRecv < 0)
        {
            perror("[readFrom] recv:");
            return 1;
        }
        else if(nRecv == 0)
        {
            perror("recv: connessione andata");
            return 1;
        }
        logM("Ricevuto per ora: %d\n", nRecv);
        memcpy(buff+i, temp, nRecv);
		i+=nRecv;
	}
		//Scrivo il contenuto nella cache:
		return writeCache(id, buff, i, pos);
}


/**
 * @brief Aggiunge un nodo alla lista di reads
 * @return 0 Se tutto e' andato bene
 * @return 1 errore
 */
int appendReadRequest(MyDFSId* id, int pos, int size)
{
	pthread_mutex_lock(id->readListMutex);

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
	pthread_mutex_unlock(id->readListMutex);

	return 0;	
}
