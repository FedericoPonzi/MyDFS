#include <stdio.h>
#include <string.h>
#include "inc/CommandsHandler.h"

#include "inc/BYE.h"
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

	if(strncmp("BYE", input, 3) == 0)
	{
		return 0;
	}
	if(strncmp("OPE", input, 3) == 0)
	{
		return 1;
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
		case 0:
			logM("[handleCommand] Ricevuta richiesta BYE \n");
			handleByeCommand(socket);
			break;
		case 1:
			logM("[handleCommand] Ricevuta richiesta OPEN\n");
			handleOpenCommand(buff, socket);
			break;
		default:
			logM("[handleCommand] Comando sconosciuto.\n");
	}
}

