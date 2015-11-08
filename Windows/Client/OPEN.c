#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

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
	toRet->indirizzo = malloc(strlen(indirizzo)+1);
	strcpy(toRet->indirizzo, indirizzo);

	//Creo il mutex:
	toRet->readListMutex = CreateMutex( NULL, FALSE, READLISTMUTEX);

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
        logM("Creato correttamente: Filename: %s, modo: %d, transferSocketId: %d, transferSock: %d\n", toRet->filename, toRet->modo, toRet->transferSocketId, toRet->controlSocketId);
    }
	return *err != 0 ? NULL : toRet;
}

/**
 * Crea la connessione di comunicazione client-server
 * Setta transferSocketId
 * 
 * WINREADY! @todo: forse si puo fare meglio, hints non serve!
 */
void createTransferSocket(MyDFSId* toRet, int *err)
{
	WSADATA wsaData;

    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;
    char buffer[30];

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    ZeroMemory( &hints, sizeof(hints) );	
    
	hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
	iResult = getaddrinfo(toRet->indirizzo, SERVER_PORT, &hints, &result);
	
	for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
    toRet->transferSocketId = socket(ptr->ai_family, ptr->ai_socktype, 
        ptr->ai_protocol);
    if (toRet->transferSocketId == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        *err = -2;
        return;
    }

        // Connect to server.
        iResult = connect( toRet->transferSocketId, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (toRet->transferSocketId == SOCKET_ERROR) {
            closesocket(toRet->transferSocketId);
            toRet->transferSocketId = INVALID_SOCKET;
            continue;
        }
        break;
	}
	
    
    
    //Connessione effettuata, invio la richiesta di apertura del file

    char openCommand [strlen(OPENCOMMAND)+strlen(toRet->filename)+5];

    sprintf(openCommand, "%s %s %d\n", OPENCOMMAND, toRet->filename, toRet->modo);
	
    if(send(toRet->transferSocketId, openCommand, sizeof(openCommand), 0) < 0)
    {
		perror("send");
		*err = -2;
		return;
	}
        
	/*
	 * Controllo per vedere se non ci sono errori nell' apertura
	 */
	if ((iResult = recv(toRet->transferSocketId, buffer, sizeof(buffer)-1, 0)) < 0)
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
	buffer[iResult]='\0';

	toRet->filesize = strtol(buffer, NULL, 10);
	
}

/**
 * @brief Crea la socket di controllo 
 *
 * Setta propriamente err se qualcosa va storto, o modifica il campo toRet->controlSocketId se tutto va bene.
 * setta controlSocketId
 * WINREady!
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

	ZeroMemory((char *) &serv_addr, sizeof(serv_addr));


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
	
	listen(sockfd, 10);
	clilen = sizeof(cli_addr);

	/* Accept actual connection from the client */
	send(toRet->transferSocketId, buffer, strlen(buffer), 0);
	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
	if (newsockfd < 0)
	{
		perror("ERROR on accept");
		*err = -2;
	}

	toRet->controlSocketId = newsockfd;
	logM("[OPEN] Aperta connessione di controllo.\n");

	if ((recv(toRet->transferSocketId, buffer, sizeof(buffer)-1, 0)) < 0)
	{
		perror("recv");
		*err = -2;
	}
	if(strncmp(buffer , "-2", 2) == 0)
	{
		*err = -2;
	}
}
