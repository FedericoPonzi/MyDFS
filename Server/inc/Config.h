#ifndef CONFIGH
#define CONFIGH

#include <pthread.h>


/**
 * @brief Numero di connessioni in ascolto
 */
 
#define BACKLOG 10

/**
 * @brief Tempo di attesa per heartbeating
 */
#define PING_TIME 2

/**
 * @brief Se in modalita' debug, stampa utili messaggi di log.
 */
#define DEBUG 1

/**
 * @brief 1 per multiprocesso, 0 per multithreading:
 */
#define MULTIPROC 1

/**
 * Dimensione dei file durante i trasferimenti
 */
#define FILESIZE 65000


/**
 * @brief Nome del file di config
 */
#define CONFIGFILENAME "CONFIG"


extern pthread_mutex_t *acceptMutex;

extern pthread_mutex_t *tempSockMutex;

extern int* numberAliveChilds;

/**
 * Numero di connessioni contemporanee permesse (equivale al numero di threads o processi)
 */
extern int numeroCon;

/**
 * E' richiesto un processo o un thread? 0 proc 1 thread
 */
extern int procOrThread;

/**
 * Numero di porta.
 */
extern int portNumber;

/**
 * root path server
 */
extern char rootPath[40];



int loadConfig();


#endif
