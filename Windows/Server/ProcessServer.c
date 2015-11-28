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

#include "inc/Config.h"
#include "inc/Utils.h"
#include "inc/CommandsHandler.h"
#include "inc/StruttureDati.h"

#include <errno.h>

DWORD dwBytes, dwMode;
WSAPROTOCOL_INFO protInfo;
SOCKET sd;

int main()
{
    printf("Hello, World from %lu\n", getptid());
    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

	LPSTR args;

	char pipeName[100];
    sprintf(pipeName, "%s%lu",PIPEPREFIX, getptid());

    HANDLE hPipe = INVALID_HANDLE_VALUE;
    
    while((hPipe = CreateFile(
	     pipeName,   // pipe name
	     GENERIC_READ |  // read and write access
	     GENERIC_WRITE,
	     0,              // no sharing
	     NULL,           // default security attributes
	     OPEN_EXISTING,  // opens existing pipe
	     0,              // default attributes
	     NULL)) == INVALID_HANDLE_VALUE)
    {
        Sleep(100);
        //Aspetto che venga creata la pipe.
    }

	
    /*dwMode = PIPE_READMODE_BYTE;

	SetNamedPipeHandleState(
		hPipe,
		&dwMode,  // new mode
		NULL,
		NULL);*/
    
    int ret;
    
    while((ret = ReadFile(hPipe, &protInfo, sizeof(protInfo), &dwBytes, NULL)) == 0 )
    {
       //do nothing
    }
 
    sd = WSASocket(AF_INET, SOCK_STREAM, 0, &protInfo, 0, WSA_FLAG_OVERLAPPED);
 
    if ( sd == INVALID_SOCKET ) 
    {
 		printf("SocketMP sock creation error - %d\n", WSAGetLastError());
        //ExitProcess(-1); 
    }
    printf("Socket: %d\n", sd);
    char storeSMHandle[1000];
    while((ret = ReadFile(hPipe, storeSMHandle, sizeof(storeSMHandle), &dwBytes, NULL)) == 0 )
    {
       //do nothing
    }

    sscanf(storeSMHandle, "%p", (HANDLE)&hSharedMemory);
    printf("hSharedMemory child address: %p\n", hSharedMemory);

    while((ret = ReadFile(hPipe, rootPath, sizeof(rootPath), &dwBytes, NULL)) == 0 )
    {
       //do nothing
    }
    
    region_sz = 1000;
    CloseHandle(hPipe);
    inizializzaPuntatori();

    handleSocket(&sd);
    
    WSACleanup();
	return 0;
}
