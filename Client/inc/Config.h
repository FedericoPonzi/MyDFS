#ifndef CONFIG
#define CONFIG
#define DEBUG 1
#define OPENCOMMAND "OPE"
#define CLOSECOMMAND "CLO"
#define READCOMMAND "REA"
#define SERVER_PORT 9001
#define BUFFSIZEREAD 65535

/**
 * @brief Tempo di attesa per heartbeating
 */
#define PING_TIME 5

typedef struct WriteOp
{
	int pos;
	int size;
	struct WriteOp* next;
} WriteOp;
typedef struct ReadOp
{
	int pos;
	int size;
	struct ReadOp* next;
} ReadOp;

typedef struct MyDFSId {  
	char* indirizzo;
	char* filename;
	int filesize;
	int modo;
	int pos;
	int socketId;
	int transferSockId;
	WriteOp* writeList;
	ReadOp* readList;
	FILE* fp;
} MyDFSId;

typedef struct CacheRequest {
	int pos;
	int size;
}CacheRequest;

    
#endif
