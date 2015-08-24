#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "inc/Utils.h"
#include "inc/Config.h"

/**
 * Stampa un messaggio di log sullo STDOUT
 * se LOG vale 0
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
}

/**
 * Stampa un errore particolare. Non interrompe il programma.
 * error: L'errore da stampare
 */
void printErr_s(char* error)
{
	printf("Error: %s", error);
}

/**
 * @brief Dice se modo_client contiene il modo "modo".
 * 
  */
int isModoApertura(int modo_client, int modo)
{
	if((modo_client&modo)==modo)
	{
		return 1;
	}
	return 0;
}


