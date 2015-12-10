/**
 * @file READ.c
 * @brief Gestisce il comando READ
 *
 * Quando il client invia un comando READ, questo modulo lo gestisce, inviando sulla socket il dato richiesto.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include "inc/Utils.h"
#include "inc/StruttureDati.h"
#include "inc/READ.h"
#include "inc/Config.h"
#include <errno.h>

int getPosFromCommand(char*);

void handleREADCommand(char* command, int socket)
{
    logM("[READ]: Ricevuta richiesta read. Command = %s\n", command);

    int nRead, nSend, pos;
	
    pos = getPosFromCommand(command);
    if(pos < 0)
    {
        logM("[READ] Error strtol");
        closeClientSession(getptid());
        close(socket);
        return;
    }
	printf("pos = %d\n", pos);

    char buff[FILESIZE];
	
	OpenedFile* id = getOpenedFile();
    FILE* fp = id->fp;
    //Mi sposto all' offset indicato dal client:
    fseek(fp, pos, SEEK_SET);
    
	nRead = fread(buff, 1, FILESIZE, fp);

	if (nRead < FILESIZE)
	{		
		if (ferror(fp))
		{
			printf("Error reading\n");
			/* Error*/
            nRead = -1;
		}
	}
    
	printf("[READ] Bytes letti: %d \n", nRead);     

    char message[15]; // strlen("size 65535") e' il massimo che mandero'.
    
    sprintf(message, "size %d", nRead);


    //Mando la dimensione della parte che ho letto
    nSend = send(socket, message, sizeof(message), 0);
    if(nSend < 0)
    {
        perror("[handleReadCommand] 1-send");
        closeClientSession(getptid());
        closesocket(socket);
        return;
    }
    else if(nRead < 0) //diamo al client la possibilita' di riprovare.
	{
        return;
    }
    //Mando la parte letta:
    nSend = send(socket, buff, nRead, 0);
    
    if(nSend<0)
    {
        perror("[handleReadCommand] 2-send");
        closeClientSession(getptid());
        closesocket(socket);
        return;
    }	
    else
    {
        logM("Inviati al client %d dati\n", nSend);
    }
}

/**
 * Ritorna la posizione della read richiesta dall' utente.
 */
int getPosFromCommand(char* command)
{
	int toRet;
    toRet = strtol(command+4, NULL, 10);
    if(errno == EINVAL || errno == ERANGE)
    {
        toRet = -1;
    }
    return toRet;
}
