#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "inc/Config.h"
#include "inc/Utils.h"
#include "inc/CommandsHandler.h"
#include "inc/StruttureDati.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

void spawnThread();
void spawnProcess();
void* handleSocket();
void spawnHeartBeat(int temp_sd);
void* heartBeat(void *temp_sd);
int sd;
struct sockaddr_in server;

/**
 * Signal handler per gestire l' invalidazione.
 */
void sig_handler(int signo)
{
    if (signo == SIGUSR1)
    {
        char* invalidate = "INVA";
        logM("received SIGUSR1\n");
        OpenedFile* id = getOpenedFile();
        if(send(id->transferSockId, invalidate, strlen(invalidate), 0) < 0)
        {
            perror("Error sending INVA\n");
        }
    }
}

/**
 * Funzione che demonizza il server.
 * Se la modalita' debug e' attiva non viene attivata la modalita' deamon.
 */
void deamonize()
{
    if(DEBUG) return;
    
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();
    
    /* An error occurred */
    if (pid < 0)
    {
        logM("Errore nella fork");
        exit(EXIT_FAILURE);
    }
    /* Success: Let the parent terminate */
    if (pid > 0)
    {
        logM("addio papy 1\n");
        exit(EXIT_SUCCESS);
    }
    /* On success: The child process becomes session leader */
    if (setsid() < 0)
    {
        logM("Errore sessione leader\n");
        exit(EXIT_FAILURE);
    }
    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
    {
        FILE* fp = fopen("Log.txt", "a");
        fprintf(fp, "Errore nella fork 2\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    /* Success: Let the parent terminate */
    if (pid > 0)
    {
        logM("Seconda fork: addio papy\n");
        exit(EXIT_SUCCESS);
    }
    /* Set new file permissions */
    umask(0);

    //chdir("/home/isaacisback/Programmazione/programmazionedisistema/Server");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
    {
        close (x);
    }

}


/**
 * Parsa l' input, e modifica le impostazioni di conseguenza.
 * ./Server [-P<port>] [-d<path>] [-c<numberofconnection>] [-h]
 *
 */
void parseInput(int argc, char* argv[])
{
    int i;
    for(i = 1; i < argc; i++)
    {
        switch(* (argv[i]+1))
        {
            case 'd':
                strcpy(rootPath, argv[i]+2);
                break;
            case 'P':
                portNumber = strtol(argv[i]+2, NULL, 10);
                break;
            case 'c':
                numeroCon = strtol(argv[i]+2, NULL, 10);
                break;
            case 'h':
                printf("Usage: ./Server [-p<port>] [-d<path>] [-c<numberofconnection>] [-h]\n");
                exit(EXIT_SUCCESS);
        }
    }
}

int main(int argc, char* argv[])
{
    //Carico il file di Config:
	loadConfig();

    //Parso l' input (ha la precedenza sul file di configurazione)
    parseInput(argc, argv);

    //Demonizzo il server
    deamonize();

	logM("[Config]\n'%d' numero di connessioni\n'%d' Processo o thread\n'%d' Porta in ascolto.\n\n", numeroCon, procOrThread, portNumber);

    //Installo il Signal Handler:
    if (signal(SIGUSR1, sig_handler) == SIG_ERR)
    {
        perror("\ncan't catch SIGUSR1\n");
        exit(EXIT_FAILURE);
    }
    
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
        perror("Socket:");
        exit(EXIT_FAILURE);
	}
    
	server.sin_family = AF_INET;
	server.sin_port = htons(portNumber);
	server.sin_addr.s_addr= INADDR_ANY;
	
	if(bind(sd, (struct sockaddr *) &server, sizeof(server)) <0)
	{
		perror("bind");
        exit(EXIT_FAILURE);
	}
	if(listen (sd, numeroCon) < 0)
	{
        perror("Listen:");
        exit(EXIT_FAILURE);
	}	

    //Pre-threaded o Pre-forked:
	while(1)
	{
		if(procOrThread)
		{
			spawnProcess();
		}
		else
		{
			spawnThread();
		}
		sleep(2);
    }

	return EXIT_SUCCESS;
}

/**
 * @brief Gestisce la comunicazione con la socket
 * 
 * Riceve un messaggio dalla socket, lo gestisce, e scrive la risposta.
 */
void* handleSocket()
{
	int temp_sd;
	struct sockaddr_in client;
	socklen_t address_size;
	address_size = sizeof(client);

	//struct per settare tempo massimo di attesa in rcv
	struct timeval tv;
    tv.tv_sec = RECV_TIMEOUT;
	tv.tv_usec = 0;

    setsockopt(temp_sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
    
	pthread_mutex_lock(acceptMutex);
	
	if((temp_sd = accept(sd, (struct sockaddr *) &client, &address_size))<0)
	{
		logM("Errore nella creazione della socket.\n");
        exit(EXIT_FAILURE);
	}

	pthread_mutex_unlock(acceptMutex);

	int nRecv;
	char buff[100];

	logM("[handleSocket] - Collegamento effettuato.\n");

	do
	{
        
		bzero(buff, sizeof(buff));
		
		nRecv = recv(temp_sd, buff, sizeof(buff)-1, 0);
		
		// Connessione chiusa o errore
		if(nRecv <= 0)
		{
			break;
		}
        
		buff[nRecv] = '\0';

		//logM("[handleSocket] - Client:'%s'\n", buff);
        		
		handleCommand(buff, temp_sd);

        logM("\n\n");
	}
	while(strncmp("CLO", buff, 3) == 0);     // Finche' non ricevo il messaggio CLO
    
	//Diminuisco il numero di figli vivi.
	*numberAliveChilds = *numberAliveChilds - 1;

    //Mi assicuro di liberare la tabella
    closeClientSession(getptid());	

	logM("[handleSocket] - Connessione terminata.\n");
		
	return NULL;
}



/**
 * @brief Crea un thread e richiama handleSocket()
 * 
 * All' arrivo di una connessione, crea un thread e richiama handleSocket
 * @see server.c#handleSocket
 * @todo dovrebbe passargli dei parametri.
 */
void spawnThread()
{
	pthread_t tid;
	while(*numberAliveChilds < numeroCon)
	{
		if (pthread_create(&tid, NULL, &handleSocket, NULL) != 0)
		{
			perror("Spawnthread:");
			exit(EXIT_FAILURE);
		}
		logM("[spawnThread] - Mio tid: %d\n", getpid());
		*numberAliveChilds = *numberAliveChilds + 1;
	}
}

/**
* @brief Crea un processo se il numero di processi disponibili e' inferiore a numeroCon
 */
void spawnProcess()
{
	pid_t pid;
	while(*numberAliveChilds < numeroCon)
	{
			
		pid = fork();
		if(pid < 0)
		{
            perror("Fork");
            exit(EXIT_FAILURE);
		}
		else if(!pid)
		{
			//Se sono il figlio:
			logM("[spawnProcess] - Mio pid: %d\n", getpid());
			handleSocket();
            exit(EXIT_SUCCESS);
		}
		else
		{
			//Se sono il padre:
			*numberAliveChilds = *numberAliveChilds + 1;
		}
	}
}
