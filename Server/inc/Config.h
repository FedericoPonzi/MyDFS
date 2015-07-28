#ifndef CONFIGH
#define CONFIGH



/**
 * @brief Numero di connessioni in ascolto
 */
 
#define BACKLOG 20

/**
 * @brief Se in modalita' debug, stampa utili messaggi di log.
 */
#define DEBUG 1

/**
 * @brief 1 per multiprocesso, 0 per multithreading:
 */
#define MULTIPROC 0

/**
 * @brief Nome del file di config
 */
#define CONFIGFILENAME "CONFIG"


extern pthread_mutex_t* acceptMutex;

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

int loadConfig();

#endif
