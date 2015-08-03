#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include "Config.h"
#include "inc/Utils.h"


int openTransferSocket();


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
	int nRecv = recv (id->transferSockId, buffer, BUFFSIZEREAD, 0);
	int k;
	for(k = pos; k < nRecv; k++)
	{
		//*(ptr+ k) = *(buffer+k);
	}
	
	
	
	return 0;
}

/**
 * @brief Apre la socket per trasferire i dati.
 * @return -1 se l' utente non ha permessi di lettura
 * 0 errore nella creazione della socket o altro
 * n la socketa.
 */
int openTransferSocket()
{
	return 0;
}
