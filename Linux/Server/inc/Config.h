#ifndef CONFIGH
#define CONFIGH

#include <pthread.h>


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
 * @brief Se in modalita' debug (=1), stampa utili messaggi di log.
 */
#define DEBUG 1

/**
 * Dimensione dei file durante i trasferimenti
 */
#define FILESIZE 65535

/**
 * @brief Nome del file di config
 */
#define CONFIGFILENAME "CONFIG"

/**
 * @brief Indica la lunghezza del buffer del nome del file.
 * @todo da quantificare bene.
 *#define FILENAME_MAX 150*/


extern pthread_mutex_t *acceptMutex;


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
