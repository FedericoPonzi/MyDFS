##My Distribuited File System
### Systems Programming exam 2014/2015

###Altri appunti:
* Sulla [open](OPE.md)
* Sulla [read](READ.md)
* Sulla [write](WRITE.md)
* Sulla [cache](Cache.md)
* Sulla [configurazione e sui test](Test.md)

###TODOS:
* Aggiungere un mutex attorno alla struttura dati (o usare quello esistente) ogni volta che necessario altrimenti genera problemi di sincoronizzazione
* Finire invalidazione heartbeating.
* Aggiungere le cazzate per il lancio.
* Alla fine: Aggingere modalità deamon.

###Più in là:
* In server.c: Spawnare gli n threads, poi creare un metodo che ogni 1 secondo (quindi sleep(1)) controlla se ci sono thread/processi da creare.

