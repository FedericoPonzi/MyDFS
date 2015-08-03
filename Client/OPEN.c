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


void createControlSocketId(MyDFSId* toRet, int *err);

void createTransferSocket(MyDFSId* toRet, int* err);

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
	//Creo la struttura di ritorno, e le aggiungo dei valori che gia conosco:
	MyDFSId* toRet;
	toRet = malloc(sizeof(toRet));
	
	toRet->indirizzo = malloc(strlen(indirizzo)+1);
	toRet->modo = modo;	
	
	strcpy(toRet->indirizzo, indirizzo);
	*err = 0;
	
	toRet->filename = malloc(strlen(nomefile)+1);
	strcpy(toRet->filename, nomefile);
	createControlSocketId(toRet, err);
	createTransferSocket(toRet, err);
	switch(*err)
	{
		case -1: 
			logM("[Open] Errore: File aperto in scrittura da un altro client");
			break;
		case -2:
			logM("[Open] Error: il server non sia raggiungibile");
			break;
		case -3:
			logM("[Open] Error: errore sul file (ad esempio file aperto in sola lettura che non esiste");
			break;
	}
	return *err != 0 ? NULL : toRet;
}

void createControlSocketId(MyDFSId* toRet, int *err)
{
    struct sockaddr_in serv_addr; 	
	if((toRet->socketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        *err= toRet->socketId;
        return;
    }
    logM("ptr = %p\n", &toRet->socketId);
    
    logM("toRet->filename = %p\n", toRet->filename);
    
    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT); 

    if(inet_aton(toRet->indirizzo, &serv_addr.sin_addr) == 0)
    {
		perror("inet_aton");
        *err= -2;
        return;
    } 
	
    if((connect(toRet->socketId, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) != 0)
    {
       *err= -2;
	   perror("connect");
       return;
    }		
    //Connessione effettuata, invio la richiesta

    char openCommand [strlen(OPENCOMMAND)+strlen(toRet->filename)+5];

    sprintf(openCommand, "%s %s %d\n", OPENCOMMAND, toRet->filename, toRet->modo);
	logM("[OpenCommand]: '%s'\n", openCommand);
	
    if(send(toRet->socketId, openCommand, sizeof(openCommand), 0) < 0)
    {
		perror("send");
		*err = -2;
		return;
	}
	char buffer[30];
	int nRecv = 0;
	if ((nRecv = recv(toRet->socketId, buffer, sizeof(buffer)-1, 0)) < 0)
	{
		perror("recv");
		*err = -2;
	}
	
	if(strncmp(buffer, "-1", 2) == 0)
	{
		*err = -1;
	}
	else if(strncmp(buffer , "-3", 2) == 0)
	{
		*err = -3;
	}
}

/**
 * @brief Crea la socket di trasporto dati
 * 
 * Setta propriamente err se qualcosa va storto, o modifica il campo toRet->transferSockId se tutto va bene.
 */
void createTransferSocket(MyDFSId* toRet, int* err)
{
	if(*err != 0) return;

	int sockfd, newsockfd;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
	if (sockfd < 0)
	{
		perror("ERROR opening socket");
		*err = -2;
	}
   
	bzero((char *) &serv_addr, sizeof(serv_addr));


	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(0);

	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR on binding");
		*err = -2;
	}
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);

	if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
	{
		*err = -2;
		perror("getsockname");
	}

	sprintf(buffer, "port_num %d", ntohs(sin.sin_port));

	send(toRet->socketId, buffer, sizeof(buffer), 0);
	 
	listen(sockfd,1);
	clilen = sizeof(cli_addr);
   
	/* Accept actual connection from the client */
	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
	if (newsockfd < 0)
	{
		perror("ERROR on accept");
		*err = -2;
	}
	
	toRet->transferSockId = newsockfd;
	logM("[OPEN] Aperta connessione di trasferimento.");


	int nRecv = 0;
	if ((nRecv = recv(toRet->socketId, buffer, sizeof(buffer)-1, 0)) < 0)
	{
		perror("recv");
		*err = -2;
	}
	
	if(strncmp(buffer , "-2", 2) == 0)
	{
		*err = -2;
	}	
}
