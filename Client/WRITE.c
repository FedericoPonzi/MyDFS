#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "inc/Config.h"
#include "inc/WRITE.h"
#include "inc/READ.h"
#include "inc/OPEN.h"
#include "inc/Utils.h"
int addWriteOp(MyDFSId* id, int pos, int size);

int writeTo(FILE* id, void* ptr, int size);

/**
 * -1 in caso di errore;
 * >= 0 ed uguale al numero di byte effettivamente scritti.
 * 
 * Pos puo essere MYSEEK_SET,MYSEEK_CUR, MYSEEK_END
 */
 
int mydfs_write(MyDFSId* id, int pos, void *ptr, unsigned int size)
{
	if(!isModoApertura(id->modo, MYO_WRONLY) && !isModoApertura(id->modo, MYO_RDWR))
	{
		logM("Non hai aperto in modo di scrittura\n");
		return -1;
	}
	//NEl caso sia end, mi sposto alla fine del file (che conosco da id->filesize)
	if(pos == MYSEEK_END)
	{
		fseek(id->fp, id->filesize, SEEK_SET);
	}
	else
	{
		fseek(id->fp, 0, pos);
	}
	int posizione = ftell(id->fp);
	//Eseguo la scrittura
	int n = writeTo(id->fp, ptr, size);	
	if(n > 0)
	{
		//In caso sia avvenuta,	
		//Aggiungo l' operazione di scrittura alla lista di operazione write
		addWriteOp(id, posizione, size);

		if(pos == MYSEEK_END)
		{
			//aggiorno la dimensione del file.
			id->filesize = id->filesize+size;
		}
	}
	return n;

}

int addWriteOp(MyDFSId* id, int pos, int size)
{
	WriteOp* writeOp = malloc(sizeof(WriteOp));
	writeOp->pos = pos;
	writeOp->size = size;
	writeOp->next = NULL;
	
	if(writeOp == NULL)
	{
		return 1;
	}		
	WriteOp* iterator = id->writeList;
	while(iterator != NULL && iterator->next != NULL)
	{
		iterator = iterator->next;
	}
	if(!iterator)
	{
		id->writeList = writeOp;
	}
	else
	{
		iterator->next = writeOp;
	}
	return 0;	
}

int writeTo(FILE* id, void* ptr, int size)
{
	return fwrite(ptr, 1, size, id);
}
