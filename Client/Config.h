#ifndef CONFIG
#define CONFIG
#define DEBUG 1
#define OPENCOMMAND "OPE"
#define CLOSECOMMAND "CLO"


typedef struct MyDFSId {  
	char* indirizzo;
	char* filename;
	int modo;
	int pos;
	int socketId;
} MyDFSId;
    
#endif
