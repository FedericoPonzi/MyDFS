#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "inc/Utils.h"
#include "inc/Config.h"

/**
 * @brief Stampa un messaggio di log sullo STDOUT
 * se DEBUG vale 1
 */

void logM (char* messaggio, ...)	
{
	if(DEBUG)
	{
        va_list args;
		va_start(args, messaggio);
		vprintf(messaggio, args);
		va_end(args);
	}
}

/**
 * @brief Stampa un errore particolare. Non interrompe il programma.
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

/**
 * @brief Ritorna il numero di cifre del numero in input.
 */
int getNumberLenght(int n)
{
	int digits = 1;

	while (n >= 10)
	{
		n /= 10;
		digits++;
	}
	return digits;
}


