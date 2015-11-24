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
#include "inc/StruttureDati.h"
#include <signal.h>




/**
 * Funzione che spawna il thread per l' heartbeating.
 */
void spawnHeartBeat(OpenedFile* id)
{
    pthread_t tid;

	if(pthread_create(&tid, NULL, &heartBeat, id) != 0)
	{
		perror("Cant create hb thread");
        myExit();
	}
}

void* heartBeat(void *args)
{
    OpenedFile* id = (OpenedFile*) args;
	int temp_sd = id->controlSocketId;
	long unsigned int ptid = id->ptid;
	logM("[Spawining HeartBeating] Tempsd: %d, ptid: %lu\n", temp_sd, ptid);
	
	//struct per settare tempo massimo di attesa in rcv
	struct timeval tv;

	tv.tv_sec = PING_TIME; 
	tv.tv_usec = 0; 
	char ping[5] = "ping";
	char pong[5];
	int nRecv;
    int nSend;
	setsockopt(temp_sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	sleep(1);
	while(1)
	{	
		//wait tot secondi
		sleep(PING_TIME);

		logM("[HB %d:%lu] PING!\n", temp_sd, ptid);
		pthread_mutex_lock(&id->controlSocketMutex);
		nSend = send(temp_sd, ping, strlen(ping)+1, 0);				
		pthread_mutex_unlock(&id->controlSocketMutex);
		if(nSend <= 0)
        {
            logM("[HB %d:%lu] Connessione terminata\n", temp_sd, ptid);
            closeClientSession(ptid);
            break;
        }
		nRecv = recv(temp_sd, pong, sizeof(pong), 0);
		if((nRecv <= 0) || (strncmp("pong", pong, 4) != 0)) //Se la socket viene chiusa o non ricevo pong esco.
		{
			logM("[heartBeat %d] - connessione %lu chiusa.\n", temp_sd, ptid);
			if(temp_sd > 0)
			{	
				closeClientSession(ptid);
				close(temp_sd);
			}
			break;
		}
		bzero(pong, sizeof(pong));
	}
	return NULL;
}





