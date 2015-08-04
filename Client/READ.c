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
int readFrom(int socketId,char  *buff, int buffsize);

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
	
	if((sizeRimasta = sendReadCommand(id,pos,ptr,size))< 0)
	{
		/* Error*/
	}
	char* buffer = malloc(BUFFSIZEREAD+1);
	if(sizeRimasta == 0)
	{
		logM("[READ] File finito di leggere.\n");
		return 0;
	}
		
	if(readFrom(id->transferSockId, buffer, sizeRimasta)) 
	{
		/* Error*/
	}
	printf("BUFFER ricevuto: %s", buffer);
	buffer[BUFFSIZEREAD] = '\0';
	strcpy(ptr, buffer);
	free(buffer);
	return 0;
}

/**
 * @brief Manda il comando di READ, e aspetta come risposta la dimensione di quanto deve leggere all'interno della risposta.
 */
int sendReadCommand(MyDFSId* id, int pos, void *ptr, unsigned int size)
{
	//Mando la read
	char readSize[2];
	sprintf(readSize, "%d", pos);
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
 
int readFrom(int socketId, char *buff, int buffsize)
{
	logM("Pronto per la read!\n");
	int nRecv = recv(socketId, buff, buffsize, 0);
	if(nRecv < 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
