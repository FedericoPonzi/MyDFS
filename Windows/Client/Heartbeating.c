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
int invalidate(MyDFSId* id); 

typedef struct ThreadArgs
{
    MyDFSId* id;
    HANDLE barriera;
}ThreadArgs; 

void spawnHeartBeat(MyDFSId* id)
{
    ThreadArgs *threadArg = (ThreadArgs*) malloc( sizeof(ThreadArgs));
    threadArg->id = id;
    threadArg->barriera = CreateSemaphore( 
        NULL,           // default security attributes
        0,  // initial count
        1,  // maximum count
        NULL);
    
	DWORD tid;
	HANDLE tHandle = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            &heartBeat,       // thread function name
            threadArg,          // argument to thread function 
            0,                      // use default creation flags 
            &tid); 
    CloseHandle(tHandle);
    WaitForSingleObject(threadArg->barriera, INFINITE);
}

/**
 * @brief Main del nuovo thread creato per l' heartbeating.
 * 
 */
void* heartBeat(void *sd)
{
    ThreadArgs* tArgs = (ThreadArgs*) sd;
    MyDFSId* id = tArgs->id;
	int controlSd = id->controlSocketId;

    logM("[Spawining HeartBeating] sd: %d \n", controlSd);
	char ping[5];
	char pong[5] = "pong";
	int nRecv;
    CloseHandle(tArgs->barriera);
    
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
            closesocket(controlSd);
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

