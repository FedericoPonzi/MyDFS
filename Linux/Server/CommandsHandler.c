#include <stdio.h>
#include <string.h>
#include "inc/CommandsHandler.h"
#include "inc/CLOSE.h"
#include "inc/OPE.h"
#include "inc/Utils.h"
#include "inc/READ.h"
#include "inc/Config.h"


/**
 * @brief Gestisce il comando ricevuto dal client.
 * 
 */
void handleCommand(char* buff, int socket)
{

	if(strncmp(OPENCOMMAND, buff, 3) == 0)
	{
		logM("[handleCommand] Ricevuta richiesta OPEN\n");
        handleOpenCommand(buff, socket);
	}
	else if(strncmp(CLOSECOMMAND, buff, 3) == 0)
	{
        logM("[handleCommand] Ricevuta richiesta CLOSE\n");
        handleCloseCommand(buff, socket);
    }
	else if(strncmp(READCOMMAND, buff, 3) == 0)
	{
        logM("[handleCommand] Ricevuta richiesta READ\n");
        handleREADCommand(buff, socket);
	}
    else
    {
        logM("[handleCommand] Comando sconosciuto.\n");
    }
}

