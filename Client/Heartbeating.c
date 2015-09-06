/**
 * Client heartbeating
 * 
 */

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
#include "inc/Heartbeating.h"
#include "inc/Config.h"
#include "inc/Utils.h"
#define BUFFSIZE 30

int invalidate(MyDFSId* id);

/**
 * spawnHeartBeat
 * @brief prende in input l' id il MyDFSId e spawna il thread heartbeating
 */
void spawnHeartBeat(MyDFSId* id)
{
	pthread_t tid;
	if(pthread_create(&tid, NULL, &heartBeat, id) != 0)
	{
		perror("Cant create hb_thread");
	}
}

/**
 * @brief Main del nuovo thread creato per l' heartbeating.
 * 
 */
void* heartBeat(void *sd)
{
	MyDFSId* id = (MyDFSId *) sd;
	int controlSd = id->socketId;
	
	logM("[Spawining HeartBeating] sd: %d \n", controlSd);
	char ping[5];
	char pong[5] = "pong";
	int nRecv;
	sleep(1);
	while(1)
	{
		nRecv = recv(controlSd, ping, strlen(ping), 0); 
		printf("ping: %s,\n", ping);
		if(nRecv > 0) //Se sono meno di zero la connessione e' chiusa.
		{
			
			if(strncmp("INVA", ping, 4) == 0)
			{
				logM("[Heartbeating] Ricevuto comando di invalidazione!");
				invalidate(id);
			}
			else if(strncmp("ping", ping, 4) == 0)
			{
				send(controlSd, pong, strlen(pong), 0);
				logM("[Heartbeating] PONG! %d, '%s'\n", strlen(ping), ping);
			}
			else
			{
				ping[nRecv] = '\0';
				logM("[Heartbeating] Ho ricevuto questo:'%s'",ping);
			}
		}
		else
		{
			logM("[heartBeat] Connessione chiusa.\n");
			return NULL;
		}
		memset(ping, 0, sizeof(ping));
	}			
	return NULL;
}

/**
 * @brief Rimuove la lista di readList a seguito della ricezione di un comando di invalidazione.
 * 
 */
int invalidate(MyDFSId* id)
{
	pthread_mutex_lock(id->readListMutex);
	ReadOp* iteratorr = id-> readList;
	ReadOp* tempr;
	while(iteratorr != NULL)
	{
		tempr = iteratorr->next;
		free(iteratorr);
		iteratorr = tempr;
	}
	iteratorr = NULL;
	pthread_mutex_unlock(id->readListMutex);	
	return 0;
}
