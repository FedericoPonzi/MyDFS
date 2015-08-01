#ifndef CONFIG
#define CONFIG
#define DEBUG 1
#define OPENCOMMAND "OPE"
#define CLOSECOMMAND "CLO"
#define SERVER_PORT 9001
#define BUFFSIZEREAD 65536

typedef struct MyDFSId {  
	char* indirizzo;
	char* filename;
	int modo;
	int pos;
	int socketId;
} MyDFSId;
    
#endif
