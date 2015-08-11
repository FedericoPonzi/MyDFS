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
	closeClientSession(getptid());
	close(socket);	
}
