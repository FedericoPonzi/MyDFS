#ifndef CONFIGH
#define CONFIGH

#include <pthread.h>

#define OPENCOMMAND "OPEN"
#define CLOSECOMMAND "CLOS"
#define READCOMMAND "READ"

/**
 * @brief Se in modalita' debug (=1), stampa utili messaggi di log.
 */
#define DEBUG 1

/**
 * @brief Numero di connessioni in ascolto
 */
 
#define BACKLOG 10

/**
 * @brief Tempo di attesa delle recv (in secondi)
 */
#define RECV_TIMEOUT 20

/**
 * @brief Tempo di attesa per heartbeating
 */
#define PING_TIME 2

/**
 * @brief Dimensione dei file durante i trasferimenti
 */
#define FILESIZE 65535

/**
 * @brief Nome del file di config
 */
#define CONFIGFILENAME "CONFIG"

/**
 * @brief Mutex dell' accept
 */
extern pthread_mutex_t *acceptMutex;

/**
 * @brief numero di processi/threads figli attualmente vivi
 */
extern int* numberAliveChilds;

/**
 * @brief Numero di connessioni contemporanee permesse (equivale al numero di threads o processi)
 */
extern int numeroCon;

/**
 * @brief E' richiesto un processo o un thread? 0 proc 1 thread
 */
extern int procOrThread;

/**
 * @brief Numero di porta.
 */
extern int portNumber;

/**
 * @brief root path server
 */
extern char rootPath[40];


int loadConfig();


#endif
