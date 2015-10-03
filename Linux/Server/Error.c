
#include "inc/Error.h"

/**
 * @brief Stampa un errore particolare. Non interrompe il programma.
 * error: L'errore da stampare
 */
void printErr_s(char* error)
{
	logM("Error: %s", error);
}

/**
 * @brief Stampa un errore in base al codice di errore. 
 * Interrompe il programma usando l' input come codice d'errore.
 * num: il codice dell' errore
 */
void printErr(int num)
{
	switch(num)
	{
			case 1:
				logM("Errore nella creazione della socket.\n");
				break;
			case 2:
				logM("Errore nella chiamata del bind.\n");
				break;
			case 3:
				logM("Errore nella chiamata di listen.\n");
				break;
			case 4:
				logM("Errore nella chiamata dell'accept.\n");
				break;
			case 5:
				logM("Errore nella chiamata della fork.\n");
				break;
			case 6:
				logM("Errore nella recv.\n");
				break;
			default:
				logM("Errore generico.\n");
	}
	exit(-num);
}
