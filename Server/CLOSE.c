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
	//-iterare gli OpenedFile
	//-per ognuno controllare che tra le SocketIdList associate compaia
	// come id "socket" in input
	//in caso negativo: skip
	//in caso positivo: rimuovere SocketIdList associata e collegare
	//					predecessore SIL rimossa con successore (next)
	//					SIL rimossa
	//Fatto ciò rimuovere tutti OpenedFile che dopo rimozione non hanno più 
	//SocketIdList associate e collegare predecessori e successori di elementi rimossi
	//chiusura connessione client-server
	logM("chiusura sessione in corso, sciao beloo\n");
	closeClientSession(socket);
	close(socket);
}
