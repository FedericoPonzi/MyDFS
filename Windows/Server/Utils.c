/**
 * @file Utils.c
 * @date 11.07.2015
 * @brief Funzioni di utilita'.
 *
 */
 
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "inc/Utils.h"
#include "inc/Config.h"
#include <stdlib.h>

/**
 * @brief Stampa un messaggio di log sullo STDOUT 
 * 
 * se LOG vale 0. Wrapper della printf.
 * @param messaggio Il messaggio da stampare
 * @param ... Le variabili
 * @see Config.h
 */
void logM (char* messaggio, ...)	
{
	va_list args;
    
	if(DEBUG)
	{
		va_start(args, messaggio);
		vprintf(messaggio, args);
		va_end(args);
	}
    else
    {
        FILE* fp = fopen("Log.txt", "a");
        fprintf(fp, messaggio, args);
        fclose(fp);
    }
}
/** * 
 * @brief Rimuove i primi 4+1 caratteri (es: 'GET ')
 * 
 * @param La linea del comando da cui vogliamo togliere il nome del comando.
 */
void stripCommand(char* input)
{
	char substringa[strlen(input)];
	strcpy(substringa, &input[5]);
	strcpy(input, substringa);
	
	if(input[strlen(input)-1] == '\n')
	{
		input[strlen(input)-1] = '\0';
	}	
}

/**
 * @brief Ritorna il numero del processo o del thread che richiama questa funzione.
 */
unsigned long int getptid()
{
	return procOrThread? GetCurrentProcessId() : GetCurrentThreadId();
}

/**
 * @brief interrompe il processo o thread a seconda del modo di avvio, a causa di un errore.
 */
void myExit()
{	
	if(procOrThread) exit(EXIT_FAILURE);
	else ExitThread(EXIT_FAILURE);
}
