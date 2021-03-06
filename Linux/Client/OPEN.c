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
 * @brief Operazione di Open
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
			logM("[Open] Errore: sembra che il server non sia raggiungibile\n");
			break;
		case -3:
			logM("[Open] Errore: errore sul file (ad esempio file aperto in sola lettura o file che non esiste)\n");
			break;
	}
    
    if(*err == 0)
    {
        logM("Creato correttamente: Filename: %s, modo: %d, transferSocketId: %d, transferSock: %d\n", toRet->filename, toRet->modo, toRet->transferSocketId, toRet->controlSocketId);
    }
	return *err != 0 ? NULL : toRet;
}

/**
 * @brief Crea la connessione di comunicazione client-server usata per lì invio di comandi e di files.
 * Setta transferSocketId
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
        perror("[createTransferSocket] inet_aton");
        *err= -2;
        return;
    }
    
    if((toRet->transferSocketId = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        *err= toRet->transferSocketId;
        return;
    }

    if((connect(toRet->transferSocketId, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) != 0)
    {
       *err= -2;
	   perror("[createTransferSocket] connect");
       return;
    }
    
    //Connessione effettuata, invio la richiesta di apertura del file

    char openCommand [strlen(OPENCOMMAND)+strlen(toRet->filename)+5]; 
    
    sprintf(openCommand, "%s %s %d\n", OPENCOMMAND, toRet->filename, toRet->modo);
	
    if(send(toRet->transferSocketId, openCommand, sizeof(openCommand), 0) < 0)
    {
		perror("[createTransferSocket] send");
		*err = -2;
		return;
	}
	
	/*
	 * Controllo per vedere se non ci sono errori nell' apertura
	 */
	if ((nRecv = recv(toRet->transferSocketId, buffer, sizeof(buffer)-1, 0)) < 0)
	{
		perror("[createTransferSocket] recv");
		*err = -2;
	}
	
	if(strncmp(buffer, "-1", 2) == 0)
	{
        logM("[createTransferSocket] Errore in apertura del file lato server 1");
		*err = -1;
		return;
	}
	else if(strncmp(buffer , "-3", 2) == 0)
	{
        logM("[createTransferSocket] Errore in apertura del file lato server 2");

		*err = -3;
		return;
	}
	buffer[nRecv]='\0';
    
	toRet->filesize = strtol(buffer, NULL, 10);
    if(errno == EINVAL || errno == ERANGE)
    {
        perror("[createTransferSocket] strtol");
        *err = -3;
    }
}

/**
 * @brief Crea la socket di controllo 
 *
 * Setta propriamente err se qualcosa va storto, o modifica il campo toRet->controlSocketId se tutto va bene.
 * setta controlSocketId
 */
void createControlSocket(MyDFSId* toRet, int* err)
{
	if(*err != 0) return;

	int sockfd, newsockfd;
	char buffer[15];
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
	{
		perror("[createControlSocket] socket");
		*err = -2;
        return;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));


	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(0);


	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("[createControlSocket] binding");
		*err = -2;
        return;
	}
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);

	if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
	{
		*err = -2;
		perror("[createControlSocket] getsockname");
        return;
	}

	logM("Sto richiedendo la connessione sulla porta numero: %d\n", ntohs(sin.sin_port));

    sprintf(buffer, "port_num %d", ntohs(sin.sin_port));
	
	if( listen(sockfd, 1) < 0)
    {
        *err = -2;
        perror("[createControlSocket] listen");
        return;
    }
    
	clilen = sizeof(cli_addr);

	if(send(toRet->transferSocketId, buffer, strlen(buffer), 0) < 0)
    {
        *err = -2;
        perror("[createControlSocket] Send");
        return;
    }

	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
	if (newsockfd < 0)
	{
		perror("[createControlSocket] accept");
		*err = -2;
        return;
	}
    close(sockfd);
    
	toRet->controlSocketId = newsockfd;

    logM("[OPEN] Aperta connessione di Controllo.\n");

	if ((recv(toRet->transferSocketId, buffer, sizeof(buffer)-1, 0)) < 0)
	{
		perror("[CreateControlSocket] recv");
		*err = -2;
        return;
	}
	if(strncmp(buffer , "-2", 2) == 0)
	{
        logM("[createControlSocket] Errori lato server");
		*err = -2;
	}
}
