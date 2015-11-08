#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <errno.h>
#include "inc/Config.h"
#include "inc/Utils.h"
#include "inc/READ.h"
#include "inc/Cache.h"

long sendReadCommand(MyDFSId* id, long pos);
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
 * WinReady!
 */
 
int mydfs_read(MyDFSId* id, int pos, void *ptr, unsigned int size)
{

	
	CacheRequest req;

	long daRicevere;
	
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
	long posizione = ftell(id->fp);

    size = posizione+size >= id->filesize ? id->filesize-posizione : size;
    logM("size: %d, filesize:%d \n", size, id->filesize);
	logM("Posizione del puntatore all' inizio della read:%d\n", posizione);
	//Mi trovo il primo buco nel file di cache:
	while(readRequest(id, posizione, size, &req))
	{
		logM("mydfs_read: primo buco: %d\n", req.pos);
		
		//Mando la richiesta di read da una certa posizione:
		daRicevere = sendReadCommand(id, req.pos);
		if(daRicevere < 0)
        {
            logM("sendReadCommand: error");
            return -1;
        }
		//Adesso nella socket ho pronti daRicevere bytes per la lettura. Me li leggo e li scrivo nel file dalla posizione
		// primoBuco
        if(readFrom(id, daRicevere, req.pos))
 		{
			logM("readFrom:Errore\n");
            return -1;
 		}
		
		appendReadRequest(id, req.pos, daRicevere); 
	}
		
	
	//Adesso nel file ho tutto il necessario, quindi eseguo una normale read:
	int n = fread(ptr, 1, size, id->fp);

	if(n<size)
	{
 		if(ferror(id->fp))
 		{
 			logM("Error nella lettura\n");
            return -1;
		}
	}
	logM("Posizione del puntatore ala fine della read:%d\n", ftell(id->fp));

	return n;
}


/**
 * @brief Manda il comando di READ, e aspetta come risposta la dimensione di quanto deve leggere all'interno della risposta.
 * WinReady
 */
long sendReadCommand(MyDFSId* id, long pos)
{
	//Mando la read

	char readCommand[strlen(READCOMMAND) + 	getNumberLenght(pos)+1]; //es: "READ 1", con 1 = la posizione
	sprintf(readCommand, "%s %li\n", READCOMMAND, pos);
	logM("Mando richiesta di READ:'%s'\n", readCommand);
	send(id->transferSocketId, readCommand, strlen(readCommand), 0);
	
	//Ricevo la dimensione della parte letta
	char fileSize[15];
	int nRecv = recv(id->transferSocketId, fileSize, sizeof(fileSize), 0);
	if(nRecv <= 0)
	{
		perror("Read command recv:");
        return -1;
	}
    
    //Metto dentro a fileSize la quantita' di byte che mi aspetto di trovare nella socket:
    fileSize[nRecv] = '\0';
    
    if(strncmp("size ", fileSize, 5) != 0)
    {
        logM("Size message Non contiene size\n");
        return -1;
    }
    
    //Tolgo "size ":
	char substringa[strlen(fileSize+5)];
	strcpy(substringa, &fileSize[5]);
	strcpy(fileSize, substringa);

    long toRet = strtol(fileSize, NULL, 10);
    if(errno == EINVAL || errno == ERANGE)
    {
        toRet = -1;
    }
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
 * Winready
 */
 
int readFrom(MyDFSId* id, int sizeRimasta,  int pos )
{
	char buff[sizeRimasta];

	int i = 0;
	int nRecv = 0;
	/*
	 * Potrei ricevere meno di sizeRimasta la prima volta. Finche' non ricevo tutti i dati continuo la recv.
	 */
	while(sizeRimasta> 0)  
	{
        logM("Size rimasta: %d", sizeRimasta);
		nRecv = recv(id->transferSocketId, buff+i, sizeRimasta, 0);
        if(nRecv < 0)
        {
            perror("[readFrom] recv:");
            return 1;
        }
        else if(nRecv == 0)
        {
            perror("[readFrom] recv: ");
            return 1;
        }
        logM("Ricevuto per ora: %d\n", nRecv);
		i+=nRecv;
        sizeRimasta -= nRecv;
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
