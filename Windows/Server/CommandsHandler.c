#include <stdio.h>
#include <string.h>
#include "inc/CommandsHandler.h"
#include "inc/CLOSE.h"
#include "inc/OPE.h"
#include "inc/Utils.h"
#include "inc/READ.h"
#include "inc/Config.h"
#include "inc/StruttureDati.h"

#include <errno.h>

/**
 * @brief Gestisce il comando ricevuto dal client.
 * 
 */
void handleCommand(char* buff, int socket)
{

	if(strncmp(OPENCOMMAND, buff, 3) == 0)
	{
		printf("[handleCommand] Ricevuta richiesta OPEN: '%s'\n", buff);
        handleOpenCommand(buff, socket);
	}
	else if(strncmp(CLOSECOMMAND, buff, 3) == 0)
	{
        printf("[handleCommand] Ricevuta richiesta CLOSE\n");
        handleCloseCommand(buff, socket);
    }
	else if(strncmp(READCOMMAND, buff, 3) == 0)
	{
        printf("[handleCommand] Ricevuta richiesta READ\n");
        handleREADCommand(buff, socket);
	}
    else
    {
        printf("[handleCommand] Comando sconosciuto.\n");
    }
}

/**
 * @brief Gestisce la comunicazione con la socket
 * 
 * Riceve un messaggio dalla socket, lo gestisce, e scrive la risposta.
 */
void* handleSocket(SOCKET *sdp)
{
	SOCKET temp_sd = INVALID_SOCKET;
	struct sockaddr_in client;
	socklen_t address_size;
    address_size = sizeof(client);
    
	WaitForSingleObject(acceptMutex, INFINITE);
    
	if((temp_sd = accept(*sdp, (struct sockaddr *) &client, &address_size)) == INVALID_SOCKET)
	{
		printf("Errore nella creazione della socket (%d).\n", WSAGetLastError());
        *numberAliveChilds = *numberAliveChilds - 1;
        ReleaseMutex(acceptMutex);

        return NULL;
        //WSACleanup();
        //myExit();
	}
	ReleaseMutex(acceptMutex);

	//struct per settare tempo massimo di attesa in rcv
	/*struct timeval tv;
    tv.tv_sec = RECV_TIMEOUT;
	tv.tv_usec = 0;

    setsockopt(temp_sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
*/

	int nRecv;
	char buff[100];

	printf("[handleSocket %lu] - Collegamento effettuato.\n", getptid());

	do
	{
        
		ZeroMemory(buff, sizeof(buff));

		nRecv = recv(temp_sd, buff, sizeof(buff)-1, 0);
		
		// Connessione chiusa o errore
		if(nRecv == 0 || nRecv == SOCKET_ERROR)
		{
            //Se nRecv = 0 e' stata chiusa bene. Altrimenti:
            if(nRecv == SOCKET_ERROR)
            {
                printf("[server] Recv Error: (%d)", WSAGetLastError());
            }
			break;
		}
        
		buff[nRecv] = '\0';
        
		handleCommand(buff, temp_sd);

        printf("\n\n");
	}
	while(strncmp(CLOSECOMMAND, buff, 3) != 0);     // Finche' non ricevo il messaggio CLO
    
	//Diminuisco il numero di figli vivi.
	*numberAliveChilds = (*numberAliveChilds) - 1;

    //Mi assicuro di liberare la tabella
    closeClientSession(getptid());	

	printf("[handleSocket] - Connessione terminata.\n");

	return NULL;
}
