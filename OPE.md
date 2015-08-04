##Alcuni appunti sulla open
Nelle specifiche:
	Per quanto riguarda il modo, devono essere supportate le seguenti modalità (per il significato vedere la documentazione sulla open Unix):
	O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_TRUNC, O_EXCL,  O_EXLOCK

* O_RDONLY: Apre il file in lettura, altri client possono richiedere lo stesso file. Se non esiste, ERRORE.
* O_WRONLY: Apre il file in scrittura. Invia un messaggio di invalidazione a tutti i client che lo hanno aperto in lettura. Se e' già aperto in scrittura, ERRORE
* O_RDWR: Apre il file in lettura e scrittura. Invia un messaggio di invalidazione a tutti i client che lo hanno aperto in lettura.
* O_CREAT: Crea il file se non esiste
* O_TRUNC: Cancella e ricrea il file sostanzialmente. Deve essere associato a O_WRONLY o O_RDWR altrimenti non ha senso.
* O_EXLOCK: Crea un lock esclusivo sul file (se è aperto in lettura nessuno può leggerlo o scriverlo). Da implementare in LINUX!
* O_EXCL: Da usare assieme a O_CREAT. Se cerca di fare la open di un file che esiste già con O_EXCL|O_CREAT allora la open fallisce.

Ogni esito và codificato e rispedito come risposta al client!

---
##Protocollo di comunicazione per la open

 1. Client: OPE file.txt 2
 2. Server: ok
 3. Client: port_num 50000
 4. Server: ok
