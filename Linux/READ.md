#Alcuni appunti sulla READ
##Funzionamento
La funzione mydfs_read funziona per ovvi motivi molto a contatto con il modulo di (Cache)[Cache.md].

Il prototipo di una mydfs_read è:


Quando un utente richiede la lettura di un file, viene fatta una richiesta alla cache per sapere se i dati richiesti dall' utente sono o meno presenti all' interno della cache.
* In caso di HIT: tutto apposto, viene effettuata una innocua read all' interno del file di cache appositamente creato. Viene aggiornato il puntatore del file (che viene usato proprio per effettuare la read) e riportata la quantità di dati richiesta dall' utente.
* In caso di MISS: la cache riempie una struct CacheReq con la posizione da cui, nel file di cache, c'è un buco che bisogna coprire per permettere all 'utente di effettuare la sua lettura. La read poi si prende carico di inviare la richiesta, e richiede alla cache la scrittura dei dati ricevuti nel file di cache. Premessi n buchi, l' n+1 richiesta alla Cache ci porterà invevitabilmente ad una HIT.


###fseek
Ref: http://www.cplusplus.com/reference/cstdio/fseek/

 * SEEK_SET Beginning of file
 * SEEK_CUR Current position of the file pointer
 * SEEK_END End of file 

