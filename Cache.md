Il modulo di caching è formato principalmente da due funzioni:

###1. File di cache
Una funzione che crea un file di caching temporaneo con nome .nomefile, che viene cancellato una volta chiuso l' ultimo file descriptor associato al file.

###2. Hit Or Miss
 Una funzione che risponde, prima di una lettura, se il dato che si vuole leggere è presente in cache o meno.
	
	int readRequest(MyDFSId* id, int pos, int size, CacheRequest* req)

Nel caso in cui il dato che il client vuole leggere non è presente in cache, la funzione ritorna come valore 0 (FALSE) e inserisce all' interno della struttura dato req:

	typedef struct CacheRequest {
		int pos;
		int size;
	}CacheRequest;
La posizione, e la dimensione del primo buco dello spazio che l' utente vuole leggere.
Quando questa funzione ritorna true, vuol dire che ho un hit completo e quindi posso procedere con la lettura.
Lo storico delle operazioni di scrittura e lettura viene mantenuto nelle rispettive liste linkate all' interno di MyDFSId.
