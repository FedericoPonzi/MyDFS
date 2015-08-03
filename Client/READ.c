#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include "Config.h"
#include "inc/Utils.h"

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
	void* buffer = malloc(BUFFSIZEREAD);
	if(sendReadCommand(id,pos,ptr,size))
	{
		/* Error*/
	}
		
	if(readFrom(id->transferSockId, buffer, size)) //todo:non size, ma la dimensione passata dal server
	{
		/* Error*/
	}
	
	return 0;
}

int sendReadCommand(MyDFSId* id, int pos, void *ptr, unsigned int size)
{

	char readSize[10];
	sprintf(readSize, "%d", size);
	char readCommand[strlen(READCOMMAND) + strlen(readSize)+2];
	sprintf(readCommand, "%s %s\n", READCOMMAND, readSize);
	send(id->socketId, readCommand, strlen(readCommand), 0);
	return 0;
}

/**
 * @brief Legge dalla socket di trasferimento buffsize dati.
 * @return 1 se c'e' un errore con la recv
 * 0 altrimenti.
 */
 
int readFrom(int socketId, char *buff, int buffsize)
{
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
