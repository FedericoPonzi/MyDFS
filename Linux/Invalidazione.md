##Invalidazione
L' invalidazione.
Quando la CLOSE del processo che comunica con uno scrittore vuole segnalare i client lettori che la loro cache non e' più valida
avrebbe bisogno di accedere alla socket di questa.
Se siamo in un ambiente multithreading questo certamente funzionerà, ma in multiprocesso non è valido in quanto ogni processo ha le sue socket associate.

###Linux
Per questo motivo, abbiamo bisogno di installare un signal handler all' interno di ogni processo (server.c) che permetta di inviare la segnalazione di invalidazione al client al momento più opportuno.

Se siamo procOrThread == 1 allora siamo in modalità processo e viene mandato un signal al processo responsabile di servire il client.
Altrimenti se procOrThread == 0 allora siamo in modalità threading e possiamo direttamente comunicare al lcient l' invalidazione.

###Windows
Ogni processo ha una pipe che usa il pid come nome, e un thread costantemente in ascolto di questa pipe che, se riceve un messaggio di invalidazione, recapiterà al client il messaggio sulla socket.

Se siamo procOrThread == 1 allora siamo in modalità processo e viene mandato un messaggio di invalidazione tramite pipe al processo responsabile di servire il client.
Altrimenti se procOrThread == 0 allora siamo in modalità threading e possiamo direttamente comunicare al lcient l' invalidazione.

###Client
Il client riceverà un messaggio sulla socket di controllo, che verrà gestito dall' heartbeating: questo prenderà possesso della lista di operazioni di lettura del client, e le cancellerà facendo risultare alla cache vuoto le parti di file già lette dal client. Quando quindi verrà richiesta la lettura di una parte del file già presente fisicamente, verrà inviata una nuova richiesta al server e aggiornato il file.

