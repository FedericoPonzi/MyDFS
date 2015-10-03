/**
 * Winready!
 */
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <unistd.h>
#include <limits.h>
#include "inc/Config.h"
#include "inc/Cache.h"
#include "inc/Utils.h"

/**
 * Winready!
 */
FILE* createTempFile(char* basename)
{
	FILE* toRet;

	TCHAR szTempFileName[MAX_PATH];  
    TCHAR lpTempPathBuffer[MAX_PATH];
    
    GetTempPath(MAX_PATH,          // length of the buffer
                           lpTempPathBuffer); // buffer for path 
    GetTempFileName(lpTempPathBuffer, // directory for tmp files
                              TEXT(basename),     // temp file name prefix 
                              0,                // create unique name 
                              szTempFileName);

	//Appena mi stacco dall' fd, viene cancellato il file temporaneo:
    printf("[Cache] Creato temp file: '%s'\n", szTempFileName);
    
	toRet = fopen(szTempFileName, "w+b");

	return toRet;
}
/**
 * @brief Usato per dire se il dato che voglio leggere si trova in cache oppure no.
 * @return 1 MISS: In caso di MISS, dentro a req verranno messi posizione e size del dato da scaricare.
 * @return 0 HIT: per interrompere il while, torna false.
 * 
 * Nota: La miss si riferisce ad un "buco", quindi bisogna chiamare la funzione piu' volte per assicurarsi che non ci siano
 * altri miss.
 * Nota2: Il lock e' in teoria troppo grande. In pratica pero', se fra la prima operazione di lettura e la seconda ci fosse una invalidazione della cache
 * qualcosa di orrendo potrebbe accadere.
 * WinReady!
 */
int readRequest(MyDFSId* id, int pos, int size, CacheRequest* req)
{
	if(pos == id->filesize)
	{
		return 0;
	}
	WriteOp* iteratorw = id->writeList;
	ReadOp* iteratorr = id->readList;
	
	while(iteratorw != NULL)
	{
		//SE il puntatore e' contenuto in una write op:
		if(iteratorw->pos <= pos && iteratorw->pos+iteratorw->size > pos)
		{
			//Controllo se e' contenuto totalmente nella write op.
			if(pos+size < iteratorw->pos+iteratorw->size)
			{
				 //Caso perfetto! total HIT! 
				 return 0;	
			}
			else
			{
				//Richiamo quest funzione spostando pos
				return readRequest(id, iteratorw->pos+iteratorw->size, size-iteratorw->size, req);
			}
		}
		iteratorw = iteratorw->next;
	}

	WaitForSingleObject(id->readListMutex, INFINITE);
	//Cerco fra le operazioni di lettura:
	while(iteratorr != NULL)
	{
		if(iteratorr->pos <= pos && iteratorr->pos+iteratorr->size > pos)
		{

			//Controllo se e' contenuto totalmente nella write op.
			if(pos+size < iteratorr->pos+iteratorr->size)
			{
				ReleaseMutex(id->readListMutex);

				 //Caso perfetto! total HIT! 
				 return 0;	
			}
			else
			{
				ReleaseMutex(id->readListMutex);

				//Richiamo quest funzione spostando pos
				return readRequest(id, iteratorr->pos+iteratorr->size, size-iteratorr->size, req); //@todo manca un argomento LOL anche la parte dopo
			}
		}
		iteratorr = iteratorr->next;
	}

	//Arrivato qui' ho dentro a pos la posizione del primo buco.
	
	
	iteratorw = id->writeList;
	iteratorr = id->readList;
	int holeSize = size;
	
	while(iteratorw != NULL)
	{
		//L' inizio del blocco contenuto in cache, deve essere compreso all' interno dell' area che sto considerando.
		if(iteratorw->pos > pos && pos+size > iteratorw->pos)
		{
			//Non solo compreso, voglio proprio l' inizio del blocco subito dopo il buco.
			holeSize = iteratorw->pos < pos + holeSize? iteratorw->pos-pos : holeSize; 
		}
		iteratorw = iteratorw->next;
	}
	while(iteratorr != NULL)
	{
		//L' inizio del blocco contenuto in cache, deve essere compreso all' interno dell' area che sto considerando.
		if(iteratorr->pos > pos && pos+size > iteratorr->pos)
		{
			//Non solo compreso, voglio proprio l' inizio del blocco subito dopo il buco.
			holeSize = iteratorr->pos < pos + holeSize?  iteratorr->pos-pos : holeSize;
		}
		iteratorr = iteratorr->next;
	}
	ReleaseMutex(id->readListMutex);

	//Se arrivo fino a qui, vuol dire che ho trovato l' inizio e la fine del buco.
	//Devo tenere in consideraione anche la dimensione del file.
	holeSize= pos + holeSize > id->filesize ? id->filesize - pos : holeSize; 
	logM("holeSize: %d\n", holeSize);
	req->size = holeSize;
	req->pos = pos;
	return 1;
}

/**
 * @brief Funzione che scrive nel file di cache il contenuto di una read, senza sovrascrivere le parti scritte da write locali.
 * 
 * Eseguo un ciclo che vede se il punto in cui:
 * 1. Controllo se il punto in cui voglio scrivere e' contenuto in una operazione di write. 
 * 		In quel caso, richiamo questa funzione spostando avanti il puntatore del buffer
 * 2. Il punto in cui voglio scrivere non e' incluso in nessuna write. In questo caso, vedo se c'e' una write inclusa nello spazio
 * 		del buffer della read che voglio scrivere. Se e' cosi', scrivo quanto posso, e se posso scrivere altro dopo la sezione
 * 		di write, richiamo questa funzione ricorsivamente aggiornado gli indici.
 * 
 * Utilizzo pos, e vedo se e' contenuto in una write. Se si lo sposto fino alla fine della write, e richiamo questa funzione ricorsivamente.
 * Raggiunto il caso base, cerco se c'e' una write contenuta fra pos e pos+size. Se e' così mi trovo la fine della write.
 * Eseguo la scrittura e richiamo questa funzione ricorsivamente
 * 
 * @return 1 se tutto e' ok, 0 se qualcosa e' andata storta		
 * 
 * WinReady!
 */
int writeCache(MyDFSId* id, void* buffer, int size, int pos)
{
	logM("[WriteCache] Devo scrivere: %d dati dalla posizione %d\n", size, pos);
	if(size <= 0)
	{
		return 0;
	}
	
	//Questa e' la write piu vicina a pos.
	WriteOp* primaWrite = NULL;
	
	WriteOp* iterator = id->writeList;
	while(iterator != NULL)
	{
		if(iterator->pos <= pos && iterator->pos+iterator->size > pos) //Se il pos e' contenuto in una write,
		{
			//Sposto il puntatore avanti e richiamo la funzione.
			int delta = iterator->pos+iterator->size - pos;
			return writeCache(id, buffer+delta, size-delta,pos+ delta);
		}
		//Se l' iterator si trova dopo pos,
		// E (se primaWrite e' null
		// Oppure iterator si trova piu vicino a pos rispetto a primaWrite)
		else if(iterator->pos-pos > 0 && (primaWrite == NULL || 
				(primaWrite->pos - pos < iterator->pos -pos)))
			{
				//Mi salvo iterator come primaWrite
				primaWrite =iterator;
			}
		iterator = iterator->next;
	}
	
	// Ora eseguo la write,
	// Se primaWrite si trova in mezzo alle scatole, scrivo fino a primaWrite e richiamo ricorsivamente questa funzione
	// Altrimento scrivo tutto il buffer che mi rimane da scrivere
	
	if(primaWrite != NULL && primaWrite->pos < pos+size) //Se primaWrite e' settato, si trova a destra di pos.
	{
		int posAttuale;
		posAttuale = ftell(id->fp);
		fseek(id->fp, pos, SEEK_SET);
		int scritti = fwrite(buffer, 1, primaWrite->pos-pos, id->fp);
		if(scritti == 0)
		{
			if(ferror(id->fp))
				return 1;
		}
		logM("[WriteCache] Scritti '%d' dati nella cache da posizione: %d.\n", scritti, primaWrite->pos-pos);
		fseek(id->fp,posAttuale, SEEK_SET);	
		// Modifico gli indici e richiamo questa funzione ricorsivamente:
		return writeCache(id, buffer+scritti, size-scritti, pos+scritti);
	}
	else
	{
		int posAttuale;
		posAttuale = ftell(id->fp);
		
		fseek(id->fp, pos, SEEK_SET);
		int s;
		if((s = fwrite(buffer, 1, size, id->fp)) == 0)
		{
			if(ferror(id->fp))
				return 1;
		}
		logM("[WriteCache] Scritti '%d' dati nella cache in posizione %d.\n", s, pos);
		fseek(id->fp,posAttuale, SEEK_SET);
		return 0;
	}
}

