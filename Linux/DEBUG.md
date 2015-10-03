##Debug dell' applicazione

###Messaggi di debug
Per il debug e' possibile impostare il flag DEBUG all' interno di Server/inc/Config.h per il server, e Client/int/Config.h per il client.
Questo stampera' durante l' esecuzione utili messaggi di debug.

###Wireshark
Permette di sniffare i pacchetti di rete, una volta avviato si esegue lo sniffing dei pacchetti sull' interfaccia di loopback (lo) in caso stiamo testando l' applicazione in locale.

###Valgrind
Usato per testare se ci sono dei memory leaks

###GDB
debugger step by step. Alcuni comandi utili:

* `set follow-fork-mode child`: Per debuggare il processo figlio di una fork o un pthread
* `r`: per avviare il debug
* `make` per ricompilare l' eseguibile
* `n`: prossima istruzione
* `b file.c:<number>`: pr bloccare l' esecuzione a una certa lnea di un file, o
* `b numefunzione`: per bloccare l' eseucizione ad una certa funzione.
