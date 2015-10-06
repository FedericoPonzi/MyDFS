#include <stdio.h>
#include <string.h>
#include "inc/CommandsHandler.h"
#include "inc/CLOSE.h"
#include "inc/OPE.h"
#include "inc/Utils.h"
#include "inc/READ.h"


/**
 * @brief Gestisce il comando ricevuto dal client.
 * 
 */
void handleCommand(char* buff, int socket)
{

	if(strncmp("OPE", buff, 3) == 0)
	{
		logM("[handleCommand] Ricevuta richiesta OPEN\n");
        handleOpenCommand(buff, socket);
	}
	else if(strncmp("CLO", buff, 3) == 0)
	{
        logM("[handleCommand] Ricevuta richiesta CLOSE\n");
        handleCloseCommand(buff, socket);
    }
	else if(strncmp("REA", buff, 3) == 0)
	{
        logM("[handleCommand] Ricevuta richiesta READ\n");
        handleREADCommand(buff, socket);
	}
    else
    {
        logM("[handleCommand] Comando sconosciuto.\n");
    }
}

