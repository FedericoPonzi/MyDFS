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


/**
 * -1 in caso di errore;
 * >= 0 ed uguale al numero di byte effettivamente scritti.
 */
 
int mydfs_write(MyDFSId* id, int pos, void *ptr, unsigned int size)
{
	//Sposto il puntatore del file pointer in base a pos.
	
	//NEl caso sia end, mi sposto alla fine del file (che conosco da id->filesize)
	if(pos == MYSEEK_END)
	{
		fseek(id->fp, id->filesize, SEEK_SET);
	}
	else
	{
		fseek(id->fp, 0, pos);
	}
	
	//Aggiungo l' operazione di scrittura alla lista di operazione write
	addWriteOp(id, pos, size);
	
	//Eseguo la scrittura
	return writeTo(id->fp, ptr, size);	
}

void addWriteOp(MyDFSId* id, int pos, int size)
{
	WriteOp* iterator = id->writeList;
	while(iterator != NULL)
	{
		iterator = iterator->next;
	}
	iterator = (WriteOp*) malloc(sizeof(WriteOp));
	iterator->pos = pos;
	iterator->size = size;
}

int writeTo(FILE* id, void* ptr, int size)
{
	return fwrite(ptr, 1, size, id);
}
