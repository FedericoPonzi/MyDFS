#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "Config.h"
#include "inc/Utils.h"
#include "inc/READ.h"

int sendReadCommand(MyDFSId* id, int pos, void *ptr, unsigned int size);
int readFrom(MyDFSId* id, int sizeRimasta);
int byteMancanti(FILE* fp);
/**
 * name: mydfs_read
 * 
 * @param id: L' id del file da cui vuole leggere.
 * @param pos: Posizione da cui vuole leggere
 * @param ptr: puntatore al buffer in cui andra a legere
 * @param size: quanto vuole leggere
 * @return -1 in caso di errore, 0 altrimenti.
 * @todo Per ora usa direttamente il buffer puntato da ptr per scrivere il risultato della read. In un futuro dovrà scrivere/leggere dalla cache.
 * 
 */
 
int mydfs_read(MyDFSId* id, int pos, void *ptr, unsigned int size)
{
	logM("[Read] Filename '%s' \n", id->filename);	
	int sizeRimasta;
	if(byteMancanti(id->fp) < 1000)
	{
		printf("Byte mancanti: '%d'\n", byteMancanti(id->fp));

		if((sizeRimasta = sendReadCommand(id,pos,ptr,size))< 0)
		{
			/* Error*/
			return -1;
		}
		if(sizeRimasta == 0)
		{
			logM("[READ] File finito di leggere.\n");
			return 0;
		}
		if(readFrom(id, sizeRimasta)) 
		{
			/* Error*/
			return -1;
		}
	}
	
	fseek(id->fp, 0, pos);
	
	int n = fread(ptr, 1, size, id->fp);
	if(n==0)
	{
		if(ferror(id->fp))
		{
			logM("Error nella lettura\n\n");
		}
	}
	return n;
}

/**
 * @brief Manda il comando di READ, e aspetta come risposta la dimensione di quanto deve leggere all'interno della risposta.
 */
int sendReadCommand(MyDFSId* id, int pos, void *ptr, unsigned int size)
{
	//Mando la read
	char readSize[2];
	sprintf(readSize, "%d", pos);
	logM("pos = %d\n", pos);
	char readCommand[strlen(READCOMMAND) + 3]; //es: "READ 1"
	sprintf(readCommand, "%s %s\n", READCOMMAND, readSize);
	send(id->socketId, readCommand, strlen(readCommand), 0);
	
	//Ricevo la dimensione della parte letta
	char fileSize[100];
	int nRecv = recv(id->socketId, fileSize, sizeof(fileSize)-1, 0);
	fileSize[nRecv] = '\0';
	logM("Messaggio ricevuto: '%s'\n", fileSize);
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
 * 0 altrimenti.
 */
 
int readFrom(MyDFSId* id, int sizeRimasta)
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
		int posAttuale;
		posAttuale = ftell(id->fp);
		fseek(id->fp,0,SEEK_END);
		fwrite(buff, nRecv, 1, id->fp);
		fseek(id->fp,posAttuale, SEEK_SET);
		return 0;
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
