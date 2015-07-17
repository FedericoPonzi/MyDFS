#include <stdio.h>
#include <string.h>
#include "inc/CommandsHandler.h"

#include "inc/CLOSE.h"
#include "inc/OPE.h"

#include "inc/Utils.h"



/**
 * Ritorna un numero corrispondente ad un comando.
 * -1 = Comando sconosciuto.
 * 0 = BYE
 * 1 = OPE
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
			handleCloseCommand(socket);
			break;
		default:
			logM("[handleCommand] Comando sconosciuto.\n");
	}
}

