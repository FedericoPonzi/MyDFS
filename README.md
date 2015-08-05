##My Distribuited File System
### Systems Programming exam 2014/2015

Tutta la documentazione si riferisce al lato server (per ora, ovviamente).
###Altri appunti:
* Sulla [open](OPE.md)
* Sulla [read](READ.md)

###TODOS:
* Aggiungere un campo lock a OpenedFile e controllare il modo alla Open
* Aggiungere heartbeating anche quando un client richiede una lock.
* Migliorare sincronizzazione tra rimozione file a fine connessione e aggiunta di nuovo file in quella successiva
* Aggiungere opzione lock alla open (e spawnare heartbeat).

###Più in là:
* In server.c: Spawnare gli n threads, poi creare un metodo che ogni 1 secondo (quindi sleep(1)) controlla se ci sono thread/processi da creare.

###Note:
* Se passo un oggetto sbagliato a logM, ricevo un errore sulla vprintf. Tipo logM("%d","ciao");
