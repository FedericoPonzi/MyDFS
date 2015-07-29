#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>


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
	logM("[Read] Filename '%s' \n", dfsid->filename);
	//char buffer[BUFFSIZEREAD];
	int nRecv;
	return recv (id->socketId, ptr, size, 0);
}

