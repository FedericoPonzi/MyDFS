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


extern int numeroCon;
extern int procOrThread;
extern int portNumber;
int loadConfig();

#endif
