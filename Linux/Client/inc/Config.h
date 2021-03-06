#ifndef CONFIG
#define CONFIG

#include <errno.h>
#include <pthread.h>

/**
 * Modalita' debug:
 */
#define DEBUG 1
#define OPENCOMMAND "OPEN"
#define CLOSECOMMAND "CLOS"
#define READCOMMAND "READ"
#define SERVER_PORT 9001
#define BUFFSIZEREAD 65535

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
	long pos;
	long size;
	struct ReadOp* next;
} ReadOp;

/**
 * @brief Contiene tutte le informazioni utili per gestire il flusso di operazioni
 */
typedef struct MyDFSId {  
	char* indirizzo; /** Indirizzo */
	char* filename; /** Nome del file */
	long filesize; /** Dimensione del file */
	int modo; /** Modo di apertura */
	int transferSocketId; /** socket id dell' heartbeating */
	int controlSocketId; /** Socket id dei comandi */
	pthread_mutex_t* readListMutex; /** Mutex per accedere la writelist */
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
