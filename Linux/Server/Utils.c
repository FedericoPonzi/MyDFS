/*
 * 
 * Copyright 2015 Federico Ponzi <federicoponzi@federicos-ubuntu>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * CHANGELOG:
 * 11.07.2015: Aggiunta di stripCommand per implementare il comando GET
 * 
 */
/**
 * @file Utils.c
 * @author Federico Ponzi
 * @date 11.07.2015
 * @brief Funzioni di utilita'.
 *
 */
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include "inc/Utils.h"
#include "inc/Config.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/unistd.h>

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
        FILE *saved = stdout;
        stdout = fopen("Log.txt", "a");

        va_start(args, messaggio);
		vprintf(messaggio, args);
		va_end(args);

        fclose(stdout);
        stdout = saved; 
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
	return procOrThread? getpid() : pthread_self();
}

/**
 * @brief interrompe il processo o thread a seconda del modo di avvio, a causa di un errore.
 */
void myExit()
{	
	if(procOrThread) pthread_exit(NULL);
	else exit(EXIT_FAILURE);
}
