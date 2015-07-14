
#include "inc/Error.h"

/**
 * Stampa un errore particolare. Non interrompe il programma.
 * error: L'errore da stampare
 */
void printErr_s(char* error)
{
	printf("Error: %s", error);
}

/**
 * Stampa un errore in base al codice di errore. 
 * Interrompe il programma usando l' input come codice d'errore.
 * num: il codice dell' errore
 */
void printErr(int num)
{
	switch(num)
	{
			case 1:
				printf("Errore nella creazione della socket.");
				break;
			case 2:
				printf("Errore nella chiamata del bind.");
				break;
			case 3:
				printf("Errore nella chiamata di listen.");
				break;
			case 4:
				printf("Errore nella chiamate dell' accept.");
				break;
			case 5:
				printf("Errore nella chiamata della fork.");
				break;
			case 6:
				printf("Errore nella recv.");
				break;
			default:
				printf("Errore generico.");
	}
	exit(-num);
}
