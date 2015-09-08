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
#include "inc/CommandsHandler.h"
#include "inc/Error.h"
#include "inc/StruttureDati.h"
#define BUFFSIZE 30


void spawnHeartBeat(int sd)
{
	pthread_t tid;
	pthreadArgs *ptarg;
	ptarg = malloc(sizeof(pthreadArgs));
	ptarg->temp_sd = sd;
	ptarg->ptid = getptid();
	
	if(pthread_create(&tid, NULL, &heartBeat, ptarg) != 0)
	{
		perror("Cant create hb thread");
	}
}

void* heartBeat(void *pt_pthreadarg)
{
	int temp_sd = ((pthreadArgs*)pt_pthreadarg)->temp_sd;
	long unsigned int ptid = ((pthreadArgs*)pt_pthreadarg)->ptid;
	logM("[Spawining HeartBeating] \n");
	logM("Tempsd: %d, ptid: %lu\n", temp_sd, ptid);
	
	//struct per settare tempo massimo di attesa in rcv
	struct timeval tv;

	tv.tv_sec = PING_TIME; 
	tv.tv_usec = 0; 
	char ping[5] = "ping";
	char pong[5];
	int nRecv;

	setsockopt(temp_sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	
	while(1)
	{	
		//wait tot secondi
		sleep(PING_TIME);	
		logM("[Heartbeating] PING!\n");
		pthread_mutex_lock(tempSockMutex);
		send(temp_sd, ping, sizeof(ping), 0);				
		pthread_mutex_unlock(tempSockMutex);
		
		nRecv = recv(temp_sd, pong, sizeof(pong), 0);
		if((nRecv < 0) || (strncmp("pong", pong, 4) != 0)) //Se la socket viene chiusa ritorna -1. quindi esco.
		{
			printf("[heartBeat] - connessione %lu chiusa: %s\n", ptid, pong);
			if(temp_sd > 0)
			{	
				closeClientSession(ptid);
				close(temp_sd);
			}
			break;
		}
		memset(pong, 0, sizeof(pong));
	}
	free(pt_pthreadarg);
	return NULL;
}


