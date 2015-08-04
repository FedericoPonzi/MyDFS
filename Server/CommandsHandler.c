#include <stdio.h>
#include <string.h>
#include "inc/CommandsHandler.h"
#include "inc/CLOSE.h"
#include "inc/OPE.h"
#include "inc/Utils.h"
#include "inc/READ.h"



/**
 * Ritorna un numero corrispondente ad un comando.
 *  @todo mergare questa funzione con quella sotto.
 */
int getCommandID(char* input)
{
	if(strncmp("OPE", input, 3) == 0)
	{
		return 1;
	}
	if(strncmp("CLO", input, 3) == 0)
	{
		return 2;
	}
	if(strncmp("REA", input, 3) == 0)
	{
		return 3;
	}
	return -1;
}


/**
 * @brief Gestisce il comando ricevuto dal client.
 * 
 */
void handleCommand(char* buff, int socket)
{
	switch(getCommandID(buff))
	{
		case 1:
			logM("[handleCommand] Ricevuta richiesta OPEN\n");
			handleOpenCommand(buff, socket);
			break;
		case 2:
			logM("[handleCommand] Ricevuta richiesta CLOSE\n");
			handleCloseCommand(socket, buff);
			break;
		case 3:
			logM("[handleCommand] Ricevuta richiesta READ\n");
			handleREADCommand(buff, socket);
			break;
		default:
			logM("[handleCommand] Comando sconosciuto.\n");
	}
}

