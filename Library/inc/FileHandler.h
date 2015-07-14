#ifndef FILEHANDLE
#define FILEHANDLE
#include <netinet/in.h>


typedef struct MyDFSId {  
	char* filename;
	char* path;
} MyDFSId;
    
// Ref:in_addr: http://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
	MyDFSId* mydfs_open(struct in_addr*, char *nomefile, int modo, int *err);


    
	int mydfs_close(MyDFSId);
	int mydfs_read(MyDFSId, int pos, void *ptr, unsigned int size);
	int mydfs_write(MyDFSId, int pos, void *ptr, unsigned int size);
#endif
