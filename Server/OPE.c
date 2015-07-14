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
 * CHANGELOG:
 * 	11.07.2015: Creazione del file.
 * 
 */
/**
 * @file OPE.c
 * @author Federico Ponzi
 * @date 11.07.2015
 * @brief Gestisce il comando OPE(N)
 *
 * Quando il client invia un comando OPEN, questo modulo lo gestisce: <br>
 * 	* Controllando che il file richiesto non sia gia' stato aperto da qualcun'altro
 *  * Se non e' aperto, aggiunge un nodo in coda a mainOpenedFile
 * 
 * @todo dovrebbe mandare una risposta al client in caso di successo o errore
 * @todo permettere l' apertura se il modo con cui e' aperto il file e' lettura e viene richiesta la lettura.
 */
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include "inc/OPE.h"
#include "inc/StruttureDati.h"
#include "inc/Utils.h"
 
void handleOpenCommand(char* command, int socket)
{
	stripCommand(command);

	char nomeFile[30];
	getFileName(command, nomeFile);
	logM("Nome del file: '%s'\n", nomeFile);
	char i = command[strlen(command)-1];

	int modo = i - '0'; //Mi da' il numero da char a int.

	logM("Modo di apertura: '%d'\n", modo);

	appendOpenedFile(nomeFile, modo, socket);
}

/**
 * @brief Torna il nome (o path del file richiesto
 * 
 * Visto che mi aspetto una stringa del tipo 'file.txt 0' allora mi basta prendere la stringa che va da 0 a n-2.
 */
void getFileName(char* command, char* nomeFile)
{

	int lunghezza = strlen(command);
	memcpy(nomeFile, command, lunghezza);
	nomeFile[lunghezza-2] = '\0';
}