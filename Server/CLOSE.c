#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "inc/OPE.h"
#include "inc/StruttureDati.h"
#include "inc/Utils.h"
#include "inc/CLOSE.h"


/**
 * @brief Gestisce il comando Close.
 * @param int socket socket del client che termina sessione
 */
void handleCloseCommand(int socket)
{	
	OpenedFile* id = getOpenedFile();
	if(isModoApertura(id->modo, MYO_RDONLY) || isModoApertura(id->modo, MYO_RDWR))
	{
		handleWrites(id);
	}
	
	closeClientSession(getptid());
	close(socket);	
}


int handleWrites(OpenedFile* id)
{
	
	return 0;
}
