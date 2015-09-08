#include <stdio.h>
#include <string.h>
#include <stdarg.h>
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


void spawnThread();
void spawnProcess();
void* handleSocket();
void spawnHeartBeat(int temp_sd);
void* heartBeat(void *temp_sd);
int sd;
struct sockaddr_in server;

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
		sleep(2);
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
	char buff[100];

	logM("[handleSocket] - Collegamento effettuato.\n");
	do
	{
		bzero(answer, sizeof(answer));
		bzero(buff, sizeof(buff));
		
		nRecv = recv(temp_sd, buff, sizeof(buff)-1, 0);
		
		// Connessione chiusa
		if(nRecv < 0 || nRecv == 0)
		{
			break;
		}
		buff[nRecv-1] = '\0';
		

		logM("[handleSocket] - Client:'%s'\n", buff);
		
		//Se ha mandato solo invio - ok e' per debugging c:
		if(strlen(buff) == 0)
		{	
			continue;
		}
		handleCommand(buff, temp_sd);
				
		logM("\n\n");
	}
	while(getCommandID(buff) != 2 && nRecv != 0); // Finche' non ricevo il messaggio BYE. o la connessione non e' chiusa
		
	//Diminuisco il numero di figli vivi.
	(*numberAliveChilds)--;
	closeClientSession(temp_sd);

	logM("[handleSocket] - Connessione terminata.\n");
		
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
	while(*numberAliveChilds < numeroCon)
	{
		if (pthread_create(&tid, NULL, &handleSocket, NULL) != 0)
		{
			printf("\n[spawnThread] - can't create thread");
			perror("Cant create thread");
			return;
		}
		logM("[spawnThread] - Mio tid: %d\n", getpid());
		(*numberAliveChilds)++;
	}
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
		//Se sono il figlio:
		logM("[spawnProcess] - Mio pid: %d\n", getpid());
		handleSocket();
	}
	else
	{
		//Se sono il padre:
		(*numberAliveChilds)++;
	}
}
