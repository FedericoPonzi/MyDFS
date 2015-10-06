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

typedef struct ThreadArgs{
    MyDFSId* id;
    pthread_barrier_t barriera;
}ThreadArgs; 

/**
 * Se il processo esegue una OPEN poi una CLOSE e poi di nuovo OPEN il sd della connessione dati della seconda connessione potrebbe coincidere col sd dell' heartbeating. L' HB potrebbe andare in recv, dopo la CLOSE e finire a fare la recv sulla connessione dati della successiva OPEN.
 */
void spawnHeartBeat(MyDFSId* id)
{
    ThreadArgs *threadArg = (ThreadArgs*) malloc( sizeof(ThreadArgs));
    threadArg->id = id;
    pthread_barrier_init(&threadArg->barriera, NULL, 2);
    
	pthread_t tid;
	if(pthread_create(&tid, NULL, &heartBeat, threadArg) != 0)
	{
		perror("Cant create hb_thread");
	}
    
    //Mi serve perche' la recv del thread dell' hb deve avvenire il prima possibile
    pthread_barrier_wait(&threadArg->barriera);     
}

/**
 * @brief Main del nuovo thread creato per l' heartbeating.
 * 
 */
void* heartBeat(void *tA)
{
    ThreadArgs* targ = (ThreadArgs * ) tA;
    
    MyDFSId* id = targ-> id;
    
	int controlSd = id->transferSockId;
    
    logM("[Spawining HeartBeating] sd: %d \n", controlSd);
	char ping[5];
	char pong[5] = "pong";
	int nRecv;

	while(1)
	{
        pthread_barrier_wait(&targ->barriera);
		nRecv = recv(controlSd, ping, sizeof(ping), 0);
        
		if(nRecv > 0)
		{
    
			if(strncmp("INVA", ping, 4) == 0)
			{
				logM("[Heartbeating %d] Ricevuto comando di invalidazione!\n", controlSd);
				invalidate(id);
			}
			else if(strncmp("ping", ping, 4) == 0)
			{
				send(controlSd, pong, strlen(pong), 0);
				logM("[Heartbeating %d] PONG! %d, '%s'\n",controlSd, strlen(ping), ping);
			}
			else
			{
				ping[nRecv] = '\0';
				logM("[Heartbeating] Ho ricevuto questo:'%s'",ping);
			}
		}
		else
		{
			logM("[HB:%d] Connessione chiusa.\n", controlSd);
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
    id->readList= NULL;
	pthread_mutex_unlock(id->readListMutex);
	return 0;
}

