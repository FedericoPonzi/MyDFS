#ifndef CONFIG
#define CONFIG


/**
 * Modalita' debug:
 */
#define DEBUG 1
#define READLISTMUTEX "readlistmutex"
#define OPENCOMMAND "OPEN"
#define CLOSECOMMAND "CLOS"
#define READCOMMAND "READ"
#define SERVER_PORT "9001"
#define BUFFSIZEREAD 65535
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>


/**
 * @brief Lista di operaioni di scrittura
 */
typedef struct WriteOp
{
	int pos;
	int size;
	struct WriteOp* next;
} WriteOp;

/**
 * @brief Lista di operazione di lettura
 */
typedef struct ReadOp
{
	int pos;
	int size;
	struct ReadOp* next;
} ReadOp;

typedef struct MyDFSId {  
	char* indirizzo;
	char* filename;
	int filesize; /** Dimensione del file */
	int modo; /** Modo di apertura */
	int pos; /** Seek_set seek_cur ecc */
	SOCKET transferSocketId; /** socket id dell' heartbeating */
	SOCKET controlSocketId; /** Socket id dei comandi */
	HANDLE readListMutex; /** Mutex per accedere la writelist */
	WriteOp* writeList; /** Lista di operazioni di scrittura avvenute nel file */
	ReadOp* readList; /**Come sopra, ma read */
	FILE* fp; /** Puntatore al file */
	
} MyDFSId;

/**
 * @brief Struttura dati usata dalla cache, per dire alla read da che posizione del file c'e' un buco.
 */
typedef struct CacheRequest {
	int pos;
	int size;
}CacheRequest;

    
#endif
