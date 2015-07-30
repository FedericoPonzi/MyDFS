#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "inc/Config.h"
#include "inc/Utils.h"
#include "inc/CommandsHandler.h"
#include "inc/Error.h"
#include "inc/StruttureDati.h"
#define BUFFSIZE 30

void spawnThread();
void spawnProcess();
void* handleSocket();
void spawnHeartBeat(int temp_sd);
void* heartBeat(void *temp_sd);
int sd;
struct sockaddr_in server;
static pthread_mutex_t *hbMutex;

int main()
{
	loadConfig();
	logM("[Config]\n'%d' numero di connessioni\n'%d' Processo o thread\n'%d' Porta in ascolto.\n\n", numeroCon, procOrThread, portNumber);


	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printErr(1);
	}	
	server.sin_family = AF_INET;
	server.sin_port = htons(portNumber);
	server.sin_addr.s_addr= INADDR_ANY;
	
	if(bind(sd, (struct sockaddr *) &server, sizeof(server)) <0)
	{
		perror("bind");
		printErr(2);
	}
	if(listen (sd, BACKLOG) < 0)
	{
		printErr(3);
	}	

	logM("Server avviato. Attendo connessioni.\n");
	// Loop infinito per servire i client:
	while(1)
	{
		if(procOrThread)
		{
			spawnProcess();
		}
		else
		{
			spawnThread();
		}
	}
	
	return 0;
}

/**
 * @brief Gestisce la comunicazione con la socket
 * 
 * Riceve un messaggio dalla socket, lo gestisce, e scrive la risposta.
 */
void* handleSocket()
{
	int temp_sd;
	struct sockaddr_in client;
	socklen_t address_size;
	address_size = sizeof(client);
	
	pthread_mutex_lock(acceptMutex);
	
	if((temp_sd = accept(sd, (struct sockaddr *) &client, &address_size))<0)
	{
		printErr(4);
	}

	pthread_mutex_unlock(acceptMutex);

	char answer[50];
	int nRecv;
	char buff[BUFFSIZE];
	
	hbMutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutexattr_t mutex_attr;
	
    if (pthread_mutexattr_init(&mutex_attr) < 0) {
        perror("Failed to initialize mutex attributes");
        exit(EXIT_FAILURE);
    }
    
    if (pthread_mutex_init(hbMutex, &mutex_attr) < 0) {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }

	logM("[handleSocket] - Collegamento effettuato.\n");
	do
	{
		bzero(answer, sizeof(answer));
		bzero(buff, sizeof(buff));
		
		nRecv = recv(temp_sd, buff, sizeof(buff)-1, 0);
		
		if(nRecv < 0)
		{
			printErr(6);
		}
		// Credo che la connessione sia chiusa.
		if(nRecv == 0)
		{
			break;
		}
		buff[nRecv-1] = '\0';
		

		logM("[handleSocket] - Client:'%s'\n", buff);
		
		if(strlen(buff) > 0)
		{	
			strcpy(answer, "\nComando Ricevuto: ");
			strcat(answer, buff);
			strcat(answer, "\n");
			
			pthread_mutex_lock(hbMutex);
			
			send(temp_sd, answer, strlen(answer), 0);
			
			pthread_mutex_unlock(hbMutex);
		}
		
		handleCommand(buff, temp_sd);
		
		OpenedFile* iterator = *openedFileLinkedList;
		while(iterator->ptid != getptid())
		{
			if(iterator->next != NULL)
			{
				iterator = iterator->next;
			}
		}
		
		if(iterator->ptid == getptid() && (isModoApertura(iterator->modo, MYO_WRONLY) || isModoApertura(iterator->modo, MYO_RDWR)))
		{
			logM("LOLOLOL\n");
			spawnHeartBeat(temp_sd);
		}		
		
		logM("\n\n");

		
	}
	while(getCommandID(buff) != 2 || nRecv != 0); // Finche' non ricevo il messaggio BYE. o la connessione non e' chiusa
		
	//Diminuisco il numero di figli vivi.
	(*numberAliveChilds)--;
	closeClientSession(temp_sd);
	logM("[handleSocket] - Connessione terminata.\n");
		
	return NULL;
}

void spawnHeartBeat(int sd)
{
	pthread_t tid;
	pthreadArgs *ptarg;
	ptarg = malloc(sizeof(pthreadArgs));
	ptarg->temp_sd = sd;
	ptarg->ptid = getptid();
	
	if(pthread_create(&tid, NULL, &heartBeat, ptarg) != 0)
	{
		perror("Cant create hb_thread");
	}
}

void* heartBeat(void *pt_pthreadarg)
{
	
	int temp_sd = ((pthreadArgs*)pt_pthreadarg)->temp_sd;
	int ptid = ((pthreadArgs*)pt_pthreadarg)->ptid;
	logM("Tempsd: %d, ptid: %d\n", temp_sd, ptid);
	//wait tot secondi
	sleep(PING_TIME);

	char ping[5] = "ping\n";
	int nRecv;
	char buff_ping_back[BUFFSIZE];
	
	pthread_mutex_lock(hbMutex);
			
	send(temp_sd, ping, strlen(ping), 0);
			
	pthread_mutex_unlock(hbMutex);
	
	//struct per settare tempo massimo di attesa in rcv
	struct timeval tv;

	tv.tv_sec = PING_TIME;  /* 5 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors

	setsockopt(temp_sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	
	nRecv = recv(temp_sd, buff_ping_back, sizeof(buff_ping_back)-1, 0); //se ci sono errori prova a levare il -1
		
	if((nRecv < 0) || (strncmp("ok", buff_ping_back, 2) != 0))
	{
		printf("[heartBeat] - connessione %d chiusa per inattività\n", ptid);
		closeClientSession(ptid);
		close(temp_sd);
	}
	free(pt_pthreadarg);
	return NULL;
}
/**
 * @brief Crea un thread e richiama handleSocket()
 * 
 * All' arrivo di una connessione, crea un thread e richiama handleSocket
 * @see server.c#handleSocket
 * @todo dovrebbe passargli dei parametri.
 */
void spawnThread()
{
	pthread_t tid;
	if(*numberAliveChilds >= numeroCon)
		return;
    if (pthread_create(&tid, NULL, &handleSocket, NULL) != 0)
    {
        printf("\n[spawnThread] - can't create thread");
        perror("Cant create thread");
	}
	(*numberAliveChilds)++;
}

/**
* @brief Crea un processo se il numero di processi disponibili e' inferiore a numeroCon
 */
void spawnProcess()
{
	pid_t pid;
	if(*numberAliveChilds >= numeroCon)
		return;
		
	pid = fork();
	if(pid < 0)
	{
		printErr(5);
	}
	else if(!pid)
	{
		printf("[spawnProcess] - Mio pid: %d\n", getpid());
		handleSocket();
	}
	else
	{
		(*numberAliveChilds)++;
	}
}
