#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "Config.h"
#include "inc/FileHandler.h"
#include "inc/OPEN.h"
#include "inc/Utils.h"

/**
 * name: mydfs_open
 * @param indirizzo: l' indirizzo ipv4 a cui vogliamo connetterci, 
 * @param nomefile: nome del file richiesto, 
 * @param modo: modo, che puo essere:  O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_TRUNC, O_EXCL,  O_EXLOCK 
 * @return NULL in caso di errore, un puntatore a MyDFSId* altrimenti.
 * 
 */

MyDFSId* mydfs_open(char* indirizzo, char *nomefile, int modo, int *err)
{
    struct sockaddr_in serv_addr; 

	//Creo la struttura di ritorno, e le aggiungo dei valori che gia conosco:
	MyDFSId* toRet;
	toRet = malloc(sizeof(toRet));
	toRet->filename = malloc(strlen(nomefile));
	strcpy(toRet->filename, nomefile);
	toRet->modo = modo;
	toRet->indirizzo = malloc(strlen(indirizzo));
	strcpy(toRet->indirizzo, indirizzo);
	
    if((toRet->socketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        *err= toRet->socketId;
        return NULL;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9000); 

    if(inet_aton(indirizzo, &serv_addr.sin_addr) == 0)
    {
		perror("inet_aton");
        printf("\n inet_pton error occured\n");
        return NULL;
    } 
	
    if( connect(toRet->socketId, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
    {
	   perror("connect");
       printf("\n Error : Connect Failed \n");
       return NULL;
    }	
    //Connessione effettuata, invio la richiesta
    char openCommand [strlen(OPENCOMMAND)+strlen(nomefile)+5];
    sprintf(openCommand, "%s %s %d\n", OPENCOMMAND, toRet->filename, toRet->modo);
	logM("[OpenCommand]: '%s'\n", openCommand);
    if(send(toRet->socketId, openCommand, sizeof(openCommand), 0) < 0)
    {
		*err = -2;
		return NULL;
	}
	char buffer[30];
	int nRecv = 0;
	if (nRecv = recv(toRet->socketId, buffer, sizeof(buffer)-1, 0) < 0)
	{
		*err = -2;
		return NULL;
	}
	if(strncmp(buffer, "ok", 2) == 0)
	{
		return toRet;
	}
	else if(strncmp(buffer, "-1", 2) == 0)
	{
		*err = -1;
	}
	else
	{
		*err = -3;
	}
	return NULL;
}



