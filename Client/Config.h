#ifndef CONFIG
#define CONFIG
#define DEBUG 1
#define OPENCOMMAND "OPE"
#define CLOSECOMMAND "CLO"
#define READCOMMAND "REA"
#define SERVER_PORT 9001
#define BUFFSIZEREAD 65535

typedef struct MyDFSId {  
	char* indirizzo;
	char* filename;
	int modo;
	int pos;
	int socketId;
	int transferSockId;
} MyDFSId;
    
#endif
