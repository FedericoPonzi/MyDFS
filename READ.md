#Alcuni appunti sulla READ
##Funzionamento
Quando viene richiesta una mydfs_open, viene eseguita una funzione che controlla quanto manca dalla posizione in cui ho richiesto la read
fino alla fine del file.
Se questa dimensione è inferiore ad un treshold, viene mandata una richiesta di read al server a partire dalla posizione n (dove n è l'ultimo byte contenuto nel file cachato).
Il nuovo pezzo di file viene letto e scritto nel file della cache.
In ogni caso, viene eseguita una lettura dal file cachato a partire dalla posizione specificata dal parametro pos (vedi più avanti fseek).


###fseek
Ref: http://www.cplusplus.com/reference/cstdio/fseek/

 * SEEK_SET Beginning of file
 * SEEK_CUR Current position of the file pointer
 * SEEK_END End of file 

