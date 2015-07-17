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
#include "Config.h"
#include "inc/Utils.h"
#include "inc/CommandsHandler.h"
#include "inc/Error.h"
#include "inc/StruttureDati.h"
#define Max 30

OpenedFile* openedFileLinkedList = NULL;
int spawnProcess(int temp_sd, int sd);
void* handleSocket(int temp_sd);

int main()
{
	struct sockaddr_in server;
	struct sockaddr_in client;
	int sd, temp_sd;
	
	socklen_t address_size;
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printErr(1);
	}	
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr= INADDR_ANY;
	
	if(bind(sd, (struct sockaddr *) &server, sizeof(server)) <0)
	{
		printErr(2);
	}
	if(listen (sd, BACKLOG) < 0)
	{
		printErr(3);
	}	
	address_size = sizeof(client);
	logM("Server avviato. Attendo connessioni. \n");
	// Loop infinito per servire i client:
	while(1)
	{
		if((temp_sd = accept(sd, (struct sockaddr *) &client, &address_size))<0)
		{
			printErr(4);
		}

		if(spawnProcess(temp_sd, sd)==0)
		{
			handleSocket(temp_sd);
			return 0;
		}
			
	}
	
	return 0;
}

/**
 * @brief Gestisce la comunicazione con la socket
 * 
 * Riceve un messaggio dalla socket, lo gestisce, e scrive la risposta.
 */
void* handleSocket(int temp_sd)
{					
	char answer[50];
	int nRecv;
	char buff[Max];

	logM("Collegamento effettuato.\n");
	do
	{
		bzero(answer, sizeof(answer));
		bzero(buff, sizeof(buff));
		printf("%d b \n\n",temp_sd);
		nRecv = recv(temp_sd, buff, sizeof(buff)-1, 0);
		if(nRecv < 0)
		{
			printErr(6);
		}
		buff[nRecv-1] = '\0';
		logM("Lunghezza di buff:'%d'\n", strlen(buff));
		logM("Client:'%s'\n", buff);
		if(strlen(buff) > 0)
		{	
			strcpy(answer, "\nComando Ricevuto: ");
			strcat(answer, buff);
			strcat(answer, "\n");
			send(temp_sd, answer, strlen(answer), 0);
		}
		
		handleCommand(buff, temp_sd);
		logM("\n\n");

		}
		while(getCommandID(buff) != 0 && nRecv != 0); // Finche' non ricevo il messaggio BYE. o la connessione non e' chiusa
		
		logM("Connessione terminata.\n");
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
	int err = 0;
	//err = pthread_create(NULL, NULL, &handleSocket, NULL);
    if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
 
}

/**
 * @brief Crea un processo e richiama handleSocket();
 * All' arrivo di una connessione, crea un processo e richiama handleSocket
 */
int spawnProcess(int temp_sd, int sd)
{
	pid_t pid;
	
	pid = fork();
	if(pid < 0)
	{
		printErr(5);
	}
	if(pid)
	{
		close(temp_sd);
	}
	else
	{
		close(sd);
	}
	return pid;
}
