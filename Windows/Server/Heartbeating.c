#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

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
    DWORD tid;
    HANDLE hThread;
	if((hThread = CreateThread(NULL,                   // default security attributes
            0,                      // use default stack size  
            (LPTHREAD_START_ROUTINE) &heartBeat,       // thread function name
            id,                   // argument to thread function 
            0,                      // use default creation flags 
            &tid)) == NULL)
	{
		perror("Cant create hb thread");
        myExit();
	}
    CloseHandle(hThread);
}

void* heartBeat(void *args)
{
    OpenedFile* id = (OpenedFile*) args;
	int controlSd = id->controlSocketId;
    SOCKET transferSocket = id->transferSocketId;
    HANDLE controlSocketMutex = id->controlSocketMutex;
	long unsigned int ptid = id->ptid;
	//printf("[Spawining HeartBeating] Tempsd: %d, ptid: %lu\n", controlSd, ptid);
	
	//struct per settare tempo massimo di attesa in rcv
	struct timeval tv;

	tv.tv_sec = PING_TIME; 
	tv.tv_usec = 0; 
	char ping[5] = "ping";
	char pong[5];
	int nRecv;
    int nSend;
	setsockopt(controlSd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

	while(1)
	{	
		//wait tot secondi
		Sleep(PING_TIME);

		printf("[HB %d:%lu] PING!\n", controlSd, ptid);
		WaitForSingleObject(controlSocketMutex, INFINITE);
		nSend = send(controlSd, ping, strlen(ping)+1, 0);				
		ReleaseMutex(controlSocketMutex);
        //Se c'è un errore, interrompo tutto.
		if(nSend <= 0)
        {
            printf("[HB %d:%lu] Connessione terminata\n", controlSd, ptid);
            CloseHandle(controlSocketMutex);
            closesocket(controlSd);
            closesocket(transferSocket);
            break;
        }
		nRecv = recv(controlSd, pong, sizeof(pong), 0);
		if((nRecv <= 0) || (strncmp("pong", pong, 4) != 0)) //Se la socket viene chiusa o non ricevo pong esco.
		{
			printf("[heartBeat %d] - connessione %lu chiusa.\n", controlSd, ptid);
            CloseHandle(controlSocketMutex);
            closesocket(controlSd);
            //closesocket(transferSocket);
			break;
		}
		ZeroMemory(pong, sizeof(pong));
	}
	return NULL;
}





