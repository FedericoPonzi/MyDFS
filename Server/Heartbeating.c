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
		perror("Cant create hb_thread");
	}
}

void* heartBeat(void *pt_pthreadarg)
{
	int temp_sd = ((pthreadArgs*)pt_pthreadarg)->temp_sd;
	int ptid = ((pthreadArgs*)pt_pthreadarg)->ptid;
	logM("[Spawining HeartBeating] \n");
	logM("Tempsd: %d, ptid: %d\n", temp_sd, ptid);
	//wait tot secondi
	sleep(PING_TIME);

	char ping[5] = "ping\n";
	int nRecv;
	char buff_ping_back[BUFFSIZE];
	
	pthread_mutex_lock(tempSockMutex);
			
	send(temp_sd, ping, strlen(ping), 0);
			
	pthread_mutex_unlock(tempSockMutex);
	
	//struct per settare tempo massimo di attesa in rcv
	struct timeval tv;

	tv.tv_sec = PING_TIME; 
	tv.tv_usec = 0; 

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
