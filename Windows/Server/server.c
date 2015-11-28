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

void spawnThread();
void spawnProcess();
void spawnHeartBeat(int temp_sd);
void* heartBeat(void *temp_sd);
SOCKET sd = INVALID_SOCKET;
struct sockaddr_in server;

/**
 * Parsa l' input, e modifica le impostazioni di conseguenza.
 * ./Server [-P<port>] [-d<path>] [-c<numberofconnection>] [-h]
 *
 */
void parseInput(int argc, char* argv[])
{
    int i;
    for(i = 1; i < argc; i++)
    {
        switch(* (argv[i]+1))
        {
            case 'd':
                strcpy(rootPath, argv[i]+2);
                break;
            case 'P':
                portNumber = strtol(argv[i]+2, NULL, 10);
                break;
            case 'c':
                numeroCon = strtol(argv[i]+2, NULL, 10);
                break;
            case 'h':
                printf("Usage: ./Server [-p<port>] [-d<path>] [-c<numberofconnection>] [-h]\n");
                exit(EXIT_SUCCESS);
        }
    }
}

int main(int argc, char* argv[])
{
    //Carico il file di Config:
	loadConfig();

    //Parso l' input (ha la precedenza sul file di configurazione)
    parseInput(argc, argv);

	printf("[Config]\n'%d' numero di connessioni\n'%d' Processo o thread\n'%d' Porta in ascolto.\n\n", numeroCon, procOrThread, portNumber);

	WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Error creating the socket (%d).", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }
	server.sin_family = AF_INET;
    server.sin_port = htons(portNumber);
    server.sin_addr.s_addr= INADDR_ANY;
    if(bind(sd, (struct sockaddr *) &server, sizeof(server)) <0)
    {
        perror("bind");
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    if(listen (sd, numeroCon) == SOCKET_ERROR)
    {
        printf("Error listening the socket (%d).", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }	

	printf("Server avviato. Attendo connessioni.\n");


    //Pre-threaded o Pre-forked:
	while(1)
	{
		if(procOrThread)
		{
			spawnProcess();
		}
		else
		{
			spawnThread();
		}
		Sleep(200);
    }
	return EXIT_SUCCESS;
}





/**
 * @brief Crea un thread e richiama handleSocket()
 * 
 * All' arrivo di una connessione, crea un thread e richiama handleSocket
 * @see CommandsHandler.c#handleSocket
 */
void spawnThread()
{
   	DWORD tid;
    HANDLE hThread;
	while(*numberAliveChilds < numeroCon)
	{

		if ((hThread = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            (LPTHREAD_START_ROUTINE) &handleSocket,       // thread function name
            &sd,                   // argument to thread function 
            0,                      // use default creation flags 
            &tid)) == NULL)
		{
			printf("Spawnthread: %lu\n", GetLastError());
			exit(EXIT_FAILURE);
		}
        CloseHandle(hThread);
		printf("[spawnThread] - Mio tid: %d\n", tid);
		*numberAliveChilds = *numberAliveChilds + 1;
	}
}

OVERLAPPED ol = {0,0,0,0,NULL};

/**
 * Invia al processo appena spanwato i dati di cui ha bisogno per avviarsi.
 */
int pipeHandler(WSAPROTOCOL_INFO protInfo, HANDLE *hPipe)
 {
    DWORD dwBytes;
    int ret;

    ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    ret = ConnectNamedPipe(*hPipe, &ol);

    if( ret == 0 ) {
 
        switch( GetLastError() ) 
        { 
            case ERROR_PIPE_CONNECTED:
 
                ret = TRUE;
 
                break;
 
            case ERROR_IO_PENDING:
 
                if( WaitForSingleObject(ol.hEvent, 1024*100) == WAIT_OBJECT_0 ) 
                { 
                    DWORD dwIgnore;
 
                    ret = GetOverlappedResult(*hPipe, &ol, &dwIgnore, FALSE);
                } 
                else 
                { 
                    CancelIo(*hPipe);
                }
 
                break;
        }
    }

	CloseHandle(ol.hEvent);

	if( ret == 0 ) 
    { 
		printf("Server - error while handling pipe: %d\n", GetLastError());
        exit(EXIT_FAILURE);
 	}
 
    if( WriteFile(*hPipe, &protInfo, sizeof(protInfo), &dwBytes, NULL) == 0 )
    {
        printf("non sto scrivendo &protInfo\n");
     	exit(EXIT_FAILURE);
    }
    
    char storeSMHandle[1000];
    sprintf(storeSMHandle, "%p", hSharedMemory);

    if( WriteFile(*hPipe, storeSMHandle, sizeof(storeSMHandle), &dwBytes, NULL) == 0 )
    {
     	printf("non sto scrivendo indirizzo mutex\n");
     	exit(EXIT_FAILURE);
    }

    if(WriteFile(*hPipe, rootPath, sizeof(rootPath), &dwBytes, NULL) == 0)
    {
    	printf("non ho scritto rootPath in pipe\n");
        exit(EXIT_FAILURE);
    }

    return 0;
 }


/**
* @brief Crea un processo se il numero di processi disponibili e' inferiore a numeroCon
 */
void spawnProcess()
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    HANDLE hPipe;
    char pipeName[100];
    WSAPROTOCOL_INFO protInfo;

	while(*numberAliveChilds < numeroCon)
	{   
        hPipe = INVALID_HANDLE_VALUE;
        ZeroMemory(pipeName, sizeof(pipeName));
        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );
        

        if(CreateProcess( "ProcessServer.exe",   // Nome del programma
        NULL,        // Argomenti
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        TRUE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
        ) 
        {
            printf("[spawnProcess] - Mio pid: %lu \n", pi.dwProcessId);
            sprintf(pipeName, "%s%lu",PIPEPREFIX, pi.dwProcessId);

            // Il padre crea la named pipe che verra' anche usata dal figlio per invalidazione:
            hPipe = CreateNamedPipe(
                      pipeName,            // pid del figlio
                      PIPE_ACCESS_DUPLEX |      // read/write access
                      FILE_FLAG_OVERLAPPED,
                      PIPE_TYPE_BYTE |              // message type pipe
                      PIPE_READMODE_BYTE |          // message-read mode
                      PIPE_WAIT,                    // blocking mode
                      PIPE_UNLIMITED_INSTANCES,     // max. instances
                      1024,                      // output buffer size
                      1024,                      // input buffer size
                      1024*100,         // client time-out
                      NULL);
             if ( hPipe == INVALID_HANDLE_VALUE ) 
            {
                printf("[SpawnProcess] Errore nella creazione della pipe.\n");
                exit(EXIT_FAILURE);

            }

            //Duplico la socket:
            if(WSADuplicateSocket(sd, pi.dwProcessId, &protInfo) == SOCKET_ERROR)
            {
                printf("Server - socket duplication failed with err no. %d\n", WSAGetLastError());
                exit(EXIT_FAILURE);

            }
            
            //e la mando al figlio sulla pipe appena creata:
            pipeHandler(protInfo, &hPipe);
            *numberAliveChilds = *numberAliveChilds + 1;
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );
            CloseHandle(hPipe);

        }
        else
        {
            printf("Errore nello spawn del processo.");
            exit(EXIT_FAILURE);
        }
        
	}
}
