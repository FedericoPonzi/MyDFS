#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "inc/Config.h"
#include "inc/Cache.h"
#include "inc/Utils.h"
FILE* createTempFile(char* basename)
{
	char tempName[strlen(basename) + 8];
	int fd;
	FILE* toRet;
	sprintf(tempName, ".%s-XXXXXX",basename);
	//Mi creo il file temporaneo:
    fd = mkstemp(tempName);
    //unlink(tempName);
	//Appena mi stacco dall' fd, viene cancellato il file temporaneo:
    printf("[Cache] Creato temp file: '%s'\n", tempName);
	toRet = fdopen(fd, "w+b");
	return toRet;
}

/**
 * @brief Usato per dire se il dato che voglio leggere si trova in cache oppure no.
 * @return 1 MISS: In caso di MISS, dentro a req verranno messi posizione e size del dato da scaricare.
 * @return 0 HIT: per interrompere il while, torna false.
 * 
 * Nota: La miss si riferisce ad un "buco", quindi bisogna chiamare la funzione piu' volte per assicurarsi che non ci siano
 * altri miss.
 * @todo da terminare 
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
	
	//Cerco fra le operazioni di lettura:
	while(iteratorr != NULL)
	{
		if(iteratorr->pos <= pos && iteratorr->pos+iteratorr->size > pos)
		{

			//Controllo se e' contenuto totalmente nella write op.
			if(pos+size < iteratorr->pos+iteratorr->size)
			{
				 //Caso perfetto! total HIT! 
				 return 0;	
			}
			else
			{
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
	
	//Se arrivo fino a qui, vuol dire che ho trovato l' inizio e la fine del buco.
	//Devo tenere in consideraione anche la dimensione del file.
	holeSize= pos + holeSize > id->filesize ? id->filesize - pos : holeSize; 
	printf("holeSize: %d\n", holeSize);
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
 * @return 1 se tutto e' ok, 0 se qualcosa e' andata storta		
 */
int writeCache(MyDFSId* id, void* buffer, int size, int pos)
{
	if(size <= 0)
	{
		return 0;
	}
	
	//Questa e' la write piu vicina a pos.
	WriteOp* primaWrite = NULL;
	
	WriteOp* iterator = id->writeList;
	while(iterator != NULL)
	{
		if(iterator->pos < pos && iterator->pos+iterator->size > pos) //Se il pos e' contenuto in una write,
		{
			logM("Pos e' contenuto in una write\n");
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
	//Segmentation fault:
	if(primaWrite != NULL && primaWrite->pos < pos+size) //Se primaWrite e' settato, si trova a destra di pos.
	{
		int posAttuale;
		posAttuale = ftell(id->fp);
		fseek(id->fp, pos, SEEK_SET);
		int scritti = fwrite(buffer, pos-primaWrite->pos, 1, id->fp);
		if(scritti == 0)
		{
			if(ferror(id->fp))
				return 1;
		}
		logM("[WriteCache] Scritti '%d' dati nella cache.\n", scritti);
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
		logM("[WriteCache] Scritti '%d' dati nella cache.\n", s);
		fseek(id->fp,posAttuale, SEEK_SET);
		return 0;
	}
}

