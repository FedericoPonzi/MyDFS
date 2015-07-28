#ifndef FILEHANDLE
#define FILEHANDLE
#include <netinet/in.h>
#include "../Config.h"




	int mydfs_read(MyDFSId*, int pos, void *ptr, unsigned int size);
	int mydfs_write(MyDFSId*, int pos, void *ptr, unsigned int size);
#endif
