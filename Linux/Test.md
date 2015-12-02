##Configurazione
Il file di configurazione contiene le impostazioni per il server. Le righe che iniziano con un simbolo `#` sono considerate commenti.
Le impostazioni necessarie sono:

 * `NUMBER_OF_CONNECTION <number>`: Equivale al numero di thread o processi da spawnare. Ogni processo/thread gestisce una connessione (=mydfs_open)
 * `PROCESS_OR_THREAD <number>`: 1 per i processi, 0 per i threads.
 * `PORT_NUMBER 9001`: Numero di porta su cui il server si mette in ascolto.
 * `ROOT_PATH /tmp/`: Path del file system su cui deve lavorare il server.

###Esempio di file di configurazione

	##FILE DI CONFIG

	###
	##Numero massimo di connessioni contemporanee:
	###
	NUMBER_OF_CONNECTION 2

	###
	## 1 per usare i processi, 0 per usare i threads
	###
	PROCESS_OR_THREAD 0

	###
	## Numero di porta su cui ascoltare la connessione:
	###
	PORT_NUMBER 9001

	###
	## Root path file
	###
	ROOT_PATH /tmp/

##Esempio di programma base
Un esempio di programma minimale che esegue una open e una close e' il seguente:
    #include <stdio.h>
    #include "inc/OPEN.h"
    #include "inc/CLOSE.h"
    #include "assert.h"
    #include <string.h>
    #include "inc/Test.h"
    int main()
    {
        printf("[TEST OPEN] Iniziato \n");
        MyDFSId* fileId;
        int error = 0;
        fileId = mydfs_open(indirizzo, filename, MYO_CREAT, &error);
        assert(fileId != NULL && error == 0);
        mydfs_close(fileId);
        printf("[Test OPEN superato correttamente!]\n");
        
        return EXIT_SUCCESS;
    }

