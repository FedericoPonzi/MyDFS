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


void spawnHeartBeat(int sd)
{
	pthread_t tid;
	int* controlSd = malloc(sizeof(int));
	*controlSd = sd;
	if(pthread_create(&tid, NULL, &heartBeat, controlSd) != 0)
	{
		perror("Cant create hb_thread");
	}
}

void* heartBeat(void *sd)
{
	int controlSd = *((int*)sd);
	logM("[Spawining HeartBeating] sd: %d \n", controlSd);
	char ping[5];
	char pong[5] = "pong";
	int nRecv;
	sleep(1);
	while(1)
	{
		nRecv = recv(controlSd, ping, sizeof(ping), 0); //se ci sono errori prova a levare il -1
		
		if((nRecv > 0) || (strncmp("ping", ping, 4) == 0))
		{
			send(controlSd, pong, strlen(pong), 0);
			logM("[Heartbeating] PONG! %d, '%s'\n", strlen(ping), ping);
		}
		else
		{
			printf("[heartBeat] Connessione chiusa.\n");
			return NULL;
		}
		memset(ping, 0, sizeof(ping));
	}			
	return NULL;
}
