/**
 * WinReady!
 */
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "inc/Heartbeating.h"
#include "inc/Config.h"
#include "inc/Utils.h"
#define BUFFSIZE 30

int invalidate(MyDFSId* id); 

void spawnHeartBeat(MyDFSId* id)
{
	DWORD tid;
	CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            heartBeat,       // thread function name
            id,          // argument to thread function 
            0,                      // use default creation flags 
            &tid); 
    
}

/**
 * @brief Main del nuovo thread creato per l' heartbeating.
 * 
 */
void* heartBeat(void *sd)
{
    MyDFSId* id = (MyDFSId *) sd;
	int controlSd = id->transferSockId;

    logM("[Spawining HeartBeating] sd: %d \n", controlSd);
	char ping[5];
	char pong[5] = "pong";
	int nRecv;
	Sleep(2);
	while(1)
	{
		nRecv = recv(controlSd, ping, sizeof(ping), 0);
		
		if(nRecv > 0)
		{

			if(strncmp("INVA", ping, 4) == 0)
			{
				logM("[Heartbeating] Ricevuto comando di invalidazione!\n");
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
			logM("[HB:%d] Connessione chiusa.\n", controlSd);
			return NULL;
		}
		ZeroMemory(ping, sizeof(ping));
	}			
	return NULL;
}

/**
 * @brief Rimuove la lista di readList a seguito della ricezione di un comando di invalidazione.
 * 
 */
int invalidate(MyDFSId* id)
{
	logM("[HeartBeating] Ricevuto comando di invalidazione!\n");
	WaitForSingleObject(id->readListMutex, INFINITE);
	ReadOp* iteratorr = id-> readList;
	ReadOp* tempr;
	while(iteratorr != NULL)
	{
		tempr = iteratorr->next;
		free(iteratorr);
		iteratorr = tempr;
	}
	iteratorr = NULL;
    id->readList= NULL;
	ReleaseMutex(id->readListMutex);
	return 0;
}

