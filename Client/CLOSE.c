#include <stdio.h>
#include "inc/FileHandler.h"
#include "inc/Utils.h"
#include "Config.h"
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
		logM("[Close] %s", id->filename);
		
		char closeCommand[strlen(id->filename)+strlen(CLOSECOMMAND)];
		
		strcpy(closeCommand, CLOSECOMMAND);

		strcat(closeCommand, id->filename);
		
		if(send(id->socketId, closeCommand, strlen(closeCommand), 0) < 0)
		{
			return -1;
		}
		close(id->socketId);
		free(id->filename);
		free(id);
		
		return 0;
}
