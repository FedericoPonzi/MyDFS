#ifndef CONFIG_H
#define CONFIG_H

#define OPENCOMMAND "OPEN"
#define CLOSECOMMAND "CLOS"
#define READCOMMAND "READ"
#define INVALIDATECOMMAND "INVA"
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#define PIPEPREFIX "\\\\.\\pipe\\"

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
 * @brief Tempo di attesa per heartbeating in millisecondi
 */
#define PING_TIME 2000

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

/**
 * Numero di processi o threads attualmente vivi
 */
extern int* numberAliveChilds;

/**
 * Mutex dell' accept
 */
extern HANDLE acceptMutex;


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
extern char rootPath[FILENAME_MAX];

/**
 * Handle della memoria condivisa
 */
extern HANDLE hSharedMemory;

/**
 * Dimensione della memoria condivisa
 */
extern DWORD region_sz;

/**
 * Puntatore alla zona di memoria condivisa
 */
void *ptr;


int loadConfig();
void inizializzaPuntatori();


#endif
