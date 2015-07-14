#include <stdio.h>
#include "inc/Utils.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


/**
 * Stampa un messaggio di log sullo STDOUT
 * se LOG vale 0
 */
void logM (char* messaggio, ...)	
{
	va_list args;

	if(1)
	{
		va_start(args, messaggio);
		vprintf(messaggio, args);
		va_end(args);
	}
}

/**
 * Stampa un errore particolare. Non interrompe il programma.
 * error: L'errore da stampare
 */
void printErr_s(char* error)
{
	printf("Error: %s", error);
}
