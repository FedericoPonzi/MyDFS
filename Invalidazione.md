##Invalidazione
L' invalidazione.
Quando la CLOSE del processo che comunica con uno scrittore vuole segnalare i client lettori che la loro cache non e' più valida
avrebbe bisogno di accedere alla socket di questa.
Se siamo in un ambiente multithreading questo certamente funzionerà, ma in multiprocesso non è valido in quanto ogni processo ha le sue socket associate.

Per questo motivo, abbiamo bisogno di installare un signal handler all' interno di ogni processo (server.c) che permetta di inviare la segnalazione di invalidazione al client al momento più opportuno.

Se siamo procOrThread == 1 allora siamo in modalità processo e viene mandato un signal al processo responsabile di servire il client. Altrimenti se procOrThread == 0 allora siamo in modalità threading e possiamo direttamente comunicare al lcient l' invalidazione.