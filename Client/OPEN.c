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
#include "inc/OPEN.h"
#include "inc/Utils.h"
#include "inc/Cache.h"
#include "inc/Heartbeating.h"

static void createTransferSocket(MyDFSId* toRet, int* err);

static void createControlSocket(MyDFSId* toRet, int *err);


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
	toRet = malloc(sizeof(MyDFSId));
	toRet->readList = NULL;
	toRet->writeList = NULL;
	toRet->readListMutex = malloc(sizeof(pthread_mutex_t));
	toRet->indirizzo = malloc(strlen(indirizzo)+1);
	strcpy(toRet->indirizzo, indirizzo);
	
	
    pthread_mutexattr_t mutex_attr;
    if (pthread_mutexattr_init(&mutex_attr) < 0) {
        perror("[Failed to initialize mutex attributes");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) < 0) {
        perror("Failed to change mutex attributes");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(toRet->readListMutex, &mutex_attr) < 0) {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }
    
	toRet->modo = modo;
	
	*err = 0;

	toRet->filename = malloc(strlen(nomefile)+1);
	strcpy(toRet->filename, nomefile);
	
	toRet->fp = createTempFile(toRet->filename);
	
	createTransferSocket(toRet, err);

	if(*err == 0) createControlSocket(toRet, err);
	if(*err == 0) spawnHeartBeat(toRet);
	
	switch(*err)
	{
		case -1:
			logM("[Open] Errore: File aperto in scrittura da un altro client\n");
			break;
		case -2:
			logM("[Open] Error: il server non sia raggiungibile\n");
			break;
		case -3:
			logM("[Open] Error: errore sul file (ad esempio file aperto in sola lettura che non esiste\n");
			break;
	}
    
    if(*err == 0)
    {
        logM("Creato correttamente: Filename: %s, modo: %d, socketId: %d, transferSock: %d\n", toRet->filename, toRet->modo, toRet->socketId, toRet->transferSockId);
    }
	return *err != 0 ? NULL : toRet;
}

/**
 * Crea la connessione di comunicazione client-server usata per lì invio di comandi e di files.
 * Setta socketId
 */
void createTransferSocket(MyDFSId* toRet, int *err)
{
	char buffer[30];
	int nRecv = 0;
    struct sockaddr_in serv_addr;
		
	memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if(inet_aton(toRet->indirizzo, &serv_addr.sin_addr) == 0)
    {
		logM("inet_aton error \n");
        perror("inet_aton:");
        *err= -2;
        return;
    }
    
    if((toRet->socketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        *err= toRet->socketId;
        return;
    }

    if((connect(toRet->socketId, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) != 0)
    {
       *err= -2;
	   perror("connect");
       return;
    }
    
    //Connessione effettuata, invio la richiesta di apertura del file

    char openCommand [strlen(OPENCOMMAND)+strlen(toRet->filename)+5];

    sprintf(openCommand, "%s %s %d\n", OPENCOMMAND, toRet->filename, toRet->modo);
	
    if(send(toRet->socketId, openCommand, sizeof(openCommand), 0) < 0)
    {
		perror("send");
		*err = -2;
		return;
	}
	
	/*
	 * Controllo per vedere se non ci sono errori nell' apertura
	 */
	if ((nRecv = recv(toRet->socketId, buffer, sizeof(buffer)-1, 0)) < 0)
	{
		perror("recv");
		*err = -2;
	}
	
	if(strncmp(buffer, "-1", 2) == 0)
	{
		*err = -1;
		return;
	}
	else if(strncmp(buffer , "-3", 2) == 0)
	{
		*err = -3;
		return;
	}
	buffer[nRecv]='\0';

	toRet->filesize = strtol(buffer, NULL, 10);
    logM("Creata connessione per il trasferimento dei dati con sd: %d", toRet->socketId);
}

/**
 * @brief Crea la socket di controllo 
 *
 * Setta propriamente err se qualcosa va storto, o modifica il campo toRet->transferSockId se tutto va bene.
 * setta transferSockId
 */
void createControlSocket(MyDFSId* toRet, int* err)
{
	if(*err != 0) return;

	int sockfd, newsockfd;
	char buffer[15];
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

	logM("Sto richiedendo la connessione sulla porta numero: %d\n", ntohs(sin.sin_port));
	sprintf(buffer, "port_num %d", ntohs(sin.sin_port));
	
	listen(sockfd, 1);
	clilen = sizeof(cli_addr);

	/* Accept actual connection from the client */
	send(toRet->socketId, buffer, strlen(buffer), 0);
	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
	if (newsockfd < 0)
	{
		perror("ERROR on accept");
		*err = -2;
	}

	toRet->transferSockId = newsockfd;
	logM("[OPEN] Aperta connessione di Controllo.");

	if ((recv(toRet->socketId, buffer, sizeof(buffer)-1, 0)) < 0)
	{
		perror("recv");
		*err = -2;
	}
	if(strncmp(buffer , "-2", 2) == 0)
	{
		*err = -2;
	}
}
