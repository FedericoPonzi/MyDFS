/*
 * WINREADY
 */
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
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
 * @brief Operazione mydfs_write definita come da specifiche
 * @return -1 in caso di errore;
 * @return >= 0 ed uguale al numero di byte effettivamente scritti.
 * 
 * @param pos puo essere MYSEEK_SET,MYSEEK_CUR, MYSEEK_END
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

/**
 * @brief Aggiunge un nodo alla lista di operazioni di scrittura
 */
int addWriteOp(MyDFSId* id, int pos, int size)
{
	WriteOp* writeOp = malloc(sizeof(WriteOp));
	if(writeOp == NULL)
	{
        perror("[addWriteOp] malloc e' fallita.");
		return 1;
	}
	writeOp->pos = pos;
	writeOp->size = size;
	writeOp->next = NULL;

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
/**
 * @brief Wrapper per la scrittura su file
 */
int writeTo(FILE* id, void* ptr, int size)
{
	return fwrite(ptr, 1, size, id);
}
