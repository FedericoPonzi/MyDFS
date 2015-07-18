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
#include "inc/Utils.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "inc/Config.h"


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
}
/** * 
 * @brief Rimuove i primi 4 caratteri (es: 'GET ')
 * 
 * @param La linea del comando a cui vogliamo togliere i primi 4 caratteri.
 */
void stripCommand(char* input)
{
	char substringa[30];
	strcpy(substringa, &input[4]);
	strcpy(input, substringa);
	
	//TODO: Per le prove, da netcat ho anche un "a capo". Così lo tolgo dai piedi.
	if(input[strlen(input)-1] == '\n')
	{
		input[strlen(input)-1] = '\0';
	}	
}


