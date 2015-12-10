/**
 * @file OPE.c
 * @brief Gestisce il comando OPE(N)
 * Quando il client invia un comando OPEN, questo modulo lo gestisce: <br>
 * 	* Controllando che il file richiesto non sia gia' stato aperto da qualcun'altro
 *  * Se non e' aperto, aggiunge un nodo in coda a mainOpenedFile
 * 
 * 
 */
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "inc/OPE.h"
#include "inc/Heartbeating.h"
#include "inc/StruttureDati.h"
#include "inc/Utils.h"
#include "inc/CLOSE.h"
#include "inc/Config.h"
#include <limits.h>
#include <errno.h>

#define PRT_MSG_SIZE 14

static char* getFileNameFromCommand(char* command);
int getModo(char* command);
int createControlSock(int portNo, int transferSocketId, OpenedFile* id);

int threadInvalidazioneMain(OpenedFile* id);

/**
 * @brief Gestisce il comando Open.
 *
 */

void handleOpenCommand(char* command, int socket)
{
	stripCommand(command);
	char answer[3] = "ok";
	char filesize_msg[100];
    char prt_msg[PRT_MSG_SIZE+1];
    
	int err_code, port_num, nRecv;

	OpenedFile* id;

	char* nomeFile = getFileNameFromCommand(command);	 

    int modo = getModo(command);
    	
	if((err_code = appendOpenedFile(nomeFile, modo, &id)))
	{
		printf("[appendOpenedFile] - Errore. \n");
		sprintf(filesize_msg, "%s", ((err_code == -3) ? "-3\n" : "-1\n"));
	}
    
    if(!err_code)
    {
        //Mando la dimensione del file
        int fileSize = 0;
        char temp_path[FILENAME_MAX];
        strcpy(temp_path, rootPath);
        strcat(temp_path, nomeFile);
        
        if(access(temp_path, F_OK) != -1) //Se esiste mi calcolo la dimensione
        {
            fseek(id->fp,0,SEEK_END);
            fileSize = ftell(id->fp);
            fseek(id->fp,0, SEEK_SET);
        }
        id->filesize = fileSize;
		id->transferSocketId = socket;
		sprintf(filesize_msg, "%d", fileSize);
    }
	
	//Mando il codice di errore o ok se presente, e se c'e' un errore mi fermo.
	if(send(socket, filesize_msg, strlen(filesize_msg), 0) < 0 || err_code)
	{
		printf("Errore nell' apertura del file (o nella send)\n");
        free(nomeFile);
        shutdown(socket, SD_BOTH);
		return;
	}
    
	//server di nuovo in ascolto per fetch port number	
	if((nRecv = recv(socket, prt_msg, sizeof(prt_msg), 0)) == SOCKET_ERROR)
	{
		printf("[handleOpenCommand] - errore rcv port no\n");
        err_code = -2;
	}
	
	if(strncmp(prt_msg, "port_num", 8))
	{
		printf("[handleOpenCommand] Errore port number\n");
        err_code = -2;
	}
	else
	{
		prt_msg[nRecv] = '\0';
		port_num = strtol(prt_msg+(strlen("port_num ")), NULL, 10);
        if(errno == EINVAL || errno == ERANGE)
        {
            printf("[handleOpenCommand] Errore formato port number\n");
            err_code = -2;
        }
	}
    
	//Se c'e' gia' un errore, non devo creare la socket di controllo.
	if(err_code || (createControlSock(port_num, socket, id)) < 1)
	{
        err_code = -2;
	}

    //Se e' in modalita' scrittura, spawno l' heartbeating
    if(!err_code && (isModoApertura(modo, MYO_WRONLY) || isModoApertura(modo, MYO_RDWR)))
    {
        spawnHeartBeat(id);
    }
    else
    {
        //Avvio il thread di invalidazione solo se sono in modalita multiprocesso:
        if(procOrThread)
        {
            DWORD tid;
            HANDLE hThread;
            if((hThread = CreateThread(NULL,                   // default security attributes
                    0,                      // use default stack size  
                    (LPTHREAD_START_ROUTINE) &threadInvalidazioneMain,       // thread function name
                    id,                   // argument to thread function 
                    0,                      // use default creation flags 
                    &tid)) == NULL)
            {
                perror("Cant create hb thread");
                err_code = -2;
            }
            else
            {
                logM("Spawnato thread per invalidazione pipe.\n");
            }
            CloseHandle(hThread);
        }
            
    }

    if(err_code)
    {
        sprintf(answer, "-2"); 
    }
    printf("[OPE %lu] Sto mandando %s sulla socket %u\n", getptid(), answer, socket);
    //Provo a mandare eventuale messaggio d' errore e se c'è un errore esco.
	if(send(socket, answer, strlen(answer), 0) < 0 || err_code)
    {
        printf("[OPE %lu Errore o nella send o precedente.", getptid());
        free(nomeFile);
        shutdown(socket, SD_BOTH);
        return;
    }
    
    free(nomeFile);
    //logM("[OpenCommand] Connessione creata correttamente.[\n Filename: %s,\n Modo: %d,\n Socket: %d,\n HBsocket: %d,\n ptid: %lu.\n]", id-> fileName, id->modo, id->transferSocketId, id->controlSocketId, getptid());
}

/**
 * @brief crea connessione di controllo (heartbeating e invalidazione) lato server
 * @return 0 se c'è un errore, il socked descriptor del control socket altrimenti.
 */
int createControlSock(int portNo, int socketId, OpenedFile* file)
{
    int sd;
    socklen_t len;
    struct sockaddr_storage addr;
    char *ipstr ;

    len = sizeof(addr);
    
    if(getpeername(socketId, (struct sockaddr*)&addr, &len))
    {
        perror("GetPeerName errore.");
        return 0;
    }

    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    if((ipstr = inet_ntoa(s->sin_addr)) == NULL)
    {
        printf("ipstr e' null.\n");
        return 0;
    }
    
    struct sockaddr_in serv_addr;
        
    ZeroMemory(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_addr.s_addr = inet_addr(ipstr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNo);
            
    if(( sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return 0;
    }
    
    if((connect(sd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
       perror("connect");
       return 0;
    }

    file->controlSocketId = sd;
    
    return sd;    
}

/**
 * @brief Torna il nome (o path del file richiesto)
 * @param command definito come "nomefile.ext n"
 * Mi calcolo dalla fine il primo spazio, e uso quello come delimitatore come nome del file.
 */
char* getFileNameFromCommand(char* command)
{
	int i = strlen(command);
	while(!isspace(command[i]) || i == 0)
	{
		i--;
	}

    char* nomeFile = (char *) malloc(i+1);

    memcpy(nomeFile, command, i);

    nomeFile[i] = '\0';
    return nomeFile;
}

/**
 * @brief Ritorna il modo dalla stringa del comando
 * 
 */
int getModo(char* command)
{
	if(isspace(*(command+strlen(command)-2)))
	{
		return *(command+strlen(command)-1) - '0'; // magic
	}
	return strtol(command+strlen(command)-2, NULL, 10);
}

/**
 * Main del thread che si mette in ascolto sulla pipe del processo
 * Usato solo in multiprocesso
 */
int threadInvalidazioneMain(OpenedFile* id)
{
    
    char pipeName[100];
    sprintf(pipeName, "%s%lu",PIPEPREFIX, getptid());
    logM("pipename: %s\n", pipeName);

    HANDLE hPipe = INVALID_HANDLE_VALUE;
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
     
     if(hPipe == INVALID_HANDLE_VALUE)
     {
         logM("Errore nella apertura della pipe.");
         return -1;
     }
     else
     {
        logM("pipe aperta");
     }
     
    while(1)
    { 
        int ret;
        char buff[10];
        while((ret = ReadFile(hPipe, &buff, sizeof(buff), NULL, NULL)) == 0 )
        {
           //do nothing
        }
        if(strncmp(buff, INVALIDATECOMMAND, strlen(INVALIDATECOMMAND)) == 0)
        {
            printf("Ricevuta invalidazione.");
            WaitForSingleObject(id->controlSocketMutex, INFINITE );
            if(send(id->controlSocketId, INVALIDATECOMMAND, strlen(INVALIDATECOMMAND), 0) < 0)
            {
                perror("Error sending INVA\n");
            }
            ReleaseMutex(id->controlSocketMutex);
        }
    }
    CloseHandle(hPipe);

}
