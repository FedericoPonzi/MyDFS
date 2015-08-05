#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>

#include <pthread.h>
#include "inc/Config.h"
#include "inc/Utils.h"
#include "inc/StruttureDati.h"
#define NUMCON "NUMBER_OF_CONNECTION"
#define PROCORTRE "PROCESS_OR_THREAD"
#define PORTNUMB "PORT_NUMBER"
#define ROOTPATH "ROOT_PATH"



static void allocaEInizializzaMemoria();

/**
 * @file StruttureDati.c
 * @author Federico Ponzi
 * @date 18.07.2015
 * @brief Contiene funzioni utilizzate per caricare il file di configurazione
 * 
 * 
 */

int *numberAliveChilds;

/**
 * Mutex dell' accept
 */
pthread_mutex_t *acceptMutex;

/**
 * Mutex della socket temporanea
 */
pthread_mutex_t *tempSockMutex;


int numeroCon = 0;
int procOrThread = 0;
int portNumber = 0;
char rootPath[40];
static void handleLine(char* line);
static void initializeMutex();
/**
 * @brief Carica le impostazioni del config, ed inizializza la memoria.
 */
int loadConfig()
{
	
	char* filename = CONFIGFILENAME;
	FILE* filePointer = fopen(filename, "ru");
	char buf[256];
	while (fgets (buf, sizeof(buf), filePointer)) 
	{	
		handleLine(buf);
	}

	// Inizializzo la memoria per le strutture dati:
	allocaEInizializzaMemoria();
	initializeMutex();
	return EXIT_SUCCESS;
}

/**
 * @brief Gestisce una linea del file di config.
 * 
 * Prende in input una linea e i aspetta o un commento o una configurazione.
 */
static void handleLine(char* line)
{

	if(strlen(line) == 0 || line[0] == '\n')
	{ 
		return;
	}
	int i = 0;
	char * ptr;
	while(line[i] != '\0')
	{
		if(line[i] == '#' && line[i+1] == '#')
		{
			break;
		}
		else
		{
			if(strncmp(line, NUMCON, strlen(NUMCON)) == 0)
			{
				i = strlen(NUMCON) + 1; //Lo spazio;
				numeroCon = strtol(&line[i], &ptr, 10);
				break;
			}
			else if(strncmp(line, PROCORTRE, strlen(PROCORTRE)) == 0)
			{
				i = strlen(PROCORTRE) +1;
				procOrThread = strtol(&line[i], &ptr, 10);
				break;
			}
			else if(strncmp(line, PORTNUMB, strlen(PORTNUMB)) == 0)
			{
				i = strlen(PORTNUMB) + 1;
				portNumber = strtol(&line[i], &ptr, 10);
				break;
			}
			else if(strncmp(line, ROOTPATH, strlen(ROOTPATH)) == 0)
			{
				i = strlen(ROOTPATH) + 1;
				int count = 0;
				//*rootPath = line[i]; //giusto?
				while(line[i] != '\n')
				{
					rootPath[count] = line[i];
					count++;
					i++;
				}
				logM("root_path = %s\n", rootPath);
				break;
			}
			else
			{
				logM("[handleLine] - Errore nel file di config: '%s' ", line);
				break;
			}
		}	
	}
}


void initializeMutex()
{
	
	tempSockMutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutexattr_t mutex_attr;
	
    if (pthread_mutexattr_init(&mutex_attr) < 0) 
    {
        perror("Failed to initialize mutex attributes");
        exit(EXIT_FAILURE);
    }
    
    if (pthread_mutex_init(tempSockMutex, &mutex_attr) < 0) 
    {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }

    pthread_mutexattr_t mutex_attr2;
    if (pthread_mutexattr_init(&mutex_attr2) < 0) 
    {
        perror("Failed to initialize mutex attributes");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutexattr_setpshared(&mutex_attr2, PTHREAD_PROCESS_SHARED) < 0) 
    {
        perror("Failed to change mutex attributes");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(acceptMutex, &mutex_attr2) < 0) 
    {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }

}


/**
 * @brief Alloca la memoria necessaria per gestire le strutture dati.
 * @todo Dovrebbe trovarsi dentro al Config.c
 */
void allocaEInizializzaMemoria()
{
	void *ptr;
    size_t region_sz = 0;

    /* Space for the nodes */
    region_sz += sizeof(OpenedFile)*numeroCon;
	
	/* Space for house-keeping pointers */
    region_sz += sizeof(openedFileLinkedList)+sizeof(free_head);

	/* Space for the mutex */
    region_sz += sizeof(*mutex);
	
	/* Spazio per il mutex dell'accept e dell' int numbero figli vivi */
	region_sz += sizeof(pthread_mutex_t);
	region_sz += sizeof(int);
	
	logM("[Initialize Memory] - Sto per allocare %lu spazio.\n", region_sz);
    ptr = mmap(NULL, region_sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    if (ptr == MAP_FAILED) 
    {
        perror("mmap(2) failed");
        exit(EXIT_FAILURE);
    }

    /* Set up everything */
    
    /**Indirizzo di serverconnectionsstruct */
    
    mutex = ptr;
    acceptMutex = mutex + sizeof(pthread_mutex_t);
    numberAliveChilds =  (int* )acceptMutex + sizeof(pthread_mutex_t);
    
    free_head = (OpenedFile **) (((char *) numberAliveChilds)+sizeof(int));
   
    openedFileLinkedList = free_head+1;

    *free_head = (OpenedFile *) (openedFileLinkedList+1);

    *openedFileLinkedList = NULL;

    /* Initialize free list */
    int i;
    OpenedFile *curr;

    for (i = 0, curr = *free_head; i < numeroCon-1; i++, curr++) 
    {
        curr->next = curr+1;
	}

    curr->next = NULL;

    pthread_mutexattr_t mutex_attr;
    if (pthread_mutexattr_init(&mutex_attr) < 0) {
        perror("[Initialize Memory] - Failed to initialize mutex attributes");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) < 0) {
        perror("[Initialize Memory] - Failed to change mutex attributes");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(mutex, &mutex_attr) < 0) {
        perror("[Initialize Memory] - Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }
}
