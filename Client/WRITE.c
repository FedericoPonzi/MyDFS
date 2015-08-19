#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "inc/Config.h"
#include "inc/WRITE.h"


/**
 * -1 in caso di errore;
 * >= 0 ed uguale al numero di byte effettivamente scritti.
 */
 
int mydfs_write(MyDFSId* dfsid, int pos, void *ptr, unsigned int size)
{
	
	return 0;	
}



