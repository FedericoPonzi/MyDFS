#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "inc/Config.h"
#include "inc/Utils.h"
#include "inc/StruttureDati.h"
#define NUMCON "NUMBER_OF_CONNECTION"
#define PROCORTRE "PROCESS_OR_THREAD"
#define PORTNUMB "PORT_NUMBER"
#define ROOTPATH "ROOT_PATH"

TCHAR szName[]=TEXT("Global\\MyFileMappingObject");

static void allocaEInizializzaMemoria();

/**
 * @file StruttureDati.c
 * @brief Contiene funzioni utilizzate per caricare il file di configurazione
 * 
 * 
 */

int *numberAliveChilds;

/**
 * Mutex dell' accept
 */
HANDLE acceptMutex;
HANDLE hSharedMemory;
int numeroCon = 0;
int procOrThread = 1; //di default, processo.
DWORD region_sz = 0;
int portNumber = 0;
char rootPath[FILENAME_MAX];
static void handleLine(char* line);
static void initializeMutex();


/**
 * @brief Carica le impostazioni del config, ed inizializza la memoria.
 */
int loadConfig()
{
	
	char* filename = CONFIGFILENAME;
	FILE* filePointer = fopen(filename, "r");
	char buf[256];
    if (filePointer == NULL)
	{
		printf("Config file %s assente\n", CONFIGFILENAME);
        exit(EXIT_FAILURE);
	}
	while (fgets (buf, sizeof(buf), filePointer)) 
	{	
		handleLine(buf);
	}
    fclose(filePointer);

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
                if (numeroCon > 256) numeroCon = 256;
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
				printf("root_path = %s\n", rootPath);
				break;
			}
			else
			{
				printf("[handleLine] - Errore nel file di config: '%s' ", line);
				break;
			}
		}	
	}
}


/**
 * @brief Alloca la memoria necessaria per gestire le strutture dati. Richiama la funzione per inizializzare i puntatori.
 */
void allocaEInizializzaMemoria()
{
    
    region_sz += sizeof(OpenedFile)*numeroCon;
    region_sz += sizeof(openedFileLL);    
	/* Spazio per il mutex "mutex" */
    region_sz += sizeof(HANDLE);
	/* Spazio per il mutex "acceptMutex" e dell' int numberofiglivivi */
	region_sz += sizeof(HANDLE); //acceptMutex
	region_sz += sizeof(int); //NumeroFigliVivi
	
	printf("[Initialize Memory] - Sto per allocare %lu spazio.\n", region_sz);
	 SECURITY_ATTRIBUTES security = { 
        sizeof(security), NULL, TRUE
    };
    
    hSharedMemory = CreateFileMapping(
                 INVALID_HANDLE_VALUE,    // use paging file
                 &security,               // Handle ereditabile
                 PAGE_READWRITE| SEC_NOCACHE | SEC_COMMIT,          // read/write access
                 0,                       // maximum object size (high-order DWORD)
                 region_sz,               // maximum object size (low-order DWORD)
                 //szName
                 NULL
                 );                 // name of mapping object
    if (hSharedMemory == NULL)
    {
        printf("CreateFileMapping: %lu", GetLastError());
        exit(EXIT_FAILURE);
    }
    
    inizializzaPuntatori();
    
    /* Initialize free list */
    OpenedFile *curr;
    int offset = 0;
    int i;
    for (i=0, curr = openedFileLL; i < numeroCon; i++, curr++) 
    {
        curr->nextOffset = offset + sizeof(OpenedFile);
        curr->used = 0;
        offset = curr->nextOffset; //Mi salvo il precedente
        curr= curr + 1;
	}
    curr->nextOffset = -1;
}

/**
 * Prende il puntatore alla memoria condivisa, e inizializza tutti i rispettivi puntatori.
 * Usata dal config e dal processo appena spawnato.
 */
void inizializzaPuntatori()
{
    

    ptr = MapViewOfFile(hSharedMemory,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        region_sz);
    printf("Ptr vale: %p\n", ptr);
    if (ptr == NULL) 
    {
        perror("MapViewOfFile failed");
        CloseHandle(hSharedMemory);
        exit(EXIT_FAILURE);
    }

    /* Set up everything */
    
    mutex = ptr;
    acceptMutex = mutex + sizeof(HANDLE);
    numberAliveChilds =  (int* )acceptMutex + sizeof(HANDLE);
    
    openedFileLL = (OpenedFile*)((char*) numberAliveChilds + sizeof(int));

}

/**
 * Inizializza i mutex
 */
void initializeMutex()
{
    SECURITY_ATTRIBUTES security = { 
        sizeof(security), NULL, TRUE
    };
    acceptMutex = CreateMutex( 
        &security,              
        FALSE,             
        NULL);

    SECURITY_ATTRIBUTES securityTwo = { 
        sizeof(securityTwo), NULL, TRUE
    };    
    mutex = CreateMutex( 
        &securityTwo, 
        FALSE,
        NULL);
        
}
