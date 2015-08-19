#include <stdio.h>
#include "inc/Utils.h"
#include "inc/Config.h"
#include "inc/CLOSE.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <errno.h>

/**
 * @param dfsid: L' id del file che l' utente vuole chiudere.
 * @return -1 in caso di errore, 0 altrimenti.
 * 
 */
int mydfs_close(MyDFSId* id)
{
		logM("[Close] '%s'\n", id->filename);
		
		char closeCommand[strlen(CLOSECOMMAND)+2];
		
		sprintf(closeCommand, "%s\n", CLOSECOMMAND);
		
		if(send(id->socketId, closeCommand, strlen(closeCommand), 0) < 0)
		{
			return -1;
		}
		close(id->socketId);
		close(id->transferSockId);
		free(id->indirizzo);
		
		free(id->filename);
		free(id);
		
		return 0;
}
