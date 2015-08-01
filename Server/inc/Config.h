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
#define PING_TIME 5

/**
 * @brief Se in modalita' debug, stampa utili messaggi di log.
 */
#define DEBUG 1

/**
 * @brief 1 per multiprocesso, 0 per multithreading:
 */
#define MULTIPROC 1

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
 * E' richiesto un processo o un thread?
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
