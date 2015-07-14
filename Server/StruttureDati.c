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
 * @file StruttureDati.c
 * @author Federico Ponzi
 * @date 11.07.2015
 * @brief Contiene le funzioni di utilita' per le Strutture Dati
 *
 * Contiene tutte le funzioni utili per lavorare con le strutture dati definite nell' header di questo modulo.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inc/Utils.h"
#include "inc/StruttureDati.h"

/**
 * @brief Aggiunge l' oggetto in input alla fine della lista linkata dei file aperti OpenedFile
 * 
 * Se non esiste la radice mainFile la crea. Altrimenti scorre tutti i nodi fino ad arrivare all' elemento in coda, crea un nuovo
 * nodo con i dati in input e lo appende alla fine.
 */
void appendOpenedFile(char* nomefile, int modo, int socket)
{
	OpenedFile* prova;
	prova = (OpenedFile*) malloc(sizeof(OpenedFile));
	prova->fileName = nomefile; 
	prova->modo = modo;
	prova->socketId = socket;
	prova->next =NULL;
	if(openedFileLinkedList == NULL)
	{
		openedFileLinkedList = prova;
		logM("[OpenFile] Main non esiste. Lo creo. '%s'\n", openedFileLinkedList->fileName);
	}
	else
	{
		openedFileLinkedList->next = prova;
		openedFileLinkedList = prova;
		logM("[OpenFile] Main esiste '%s' \n", openedFileLinkedList->fileName);
		OpenedFile* iterator = openedFileLinkedList;
		while(iterator->next != NULL)
		{
			iterator = iterator->next;
		}
		iterator->next = prova;
	}
	logM("[OpenFile] Aggiunto. Nome: '%s' \n", prova->fileName);

	
}

/**
 * @brief Controlla se un file con quel nome e' gia' stato aperto.
 * @param fileName Il nome del file da controllare
 * @param socketId l' id della socket
 * 
 * @return 1 se il file non e' aperto,
 * @return 1 se il file e' gia' aperto, ed e' associato alla stessa socketId
 * @return 1 se ha richiesto l'apertura in read
 * @return 0 se il file e' gia' aperto in scrittura da altri
 */

int fileAlreadyOpen(char* fileName, int socketId, int modo)
{
	OpenedFile* iterator = openedFileLinkedList;
	printf("YO\n");
	while(iterator->next != NULL)
	{
		if(strcmp(iterator->fileName, fileName) == 0)
		{
			return TRUE;
		}
		printf("Provo: '%s' \n", iterator->fileName);
		iterator = iterator->next;
	}
	return FALSE;
	
}


/**
 * @brief Ritorna un tipo modoApertura partendo da un intero.
 * 
 * Gli enum in teoria sono codificato come 0, 1 ecc pero' qui' dice di non fare affidamento suq uesta cosa: 
 * @see http://www.cs.utah.edu/~germain/PPS/Topics/C_Language/enumerated_types.html
 */
modoApertura_t getModoAperturaFromInt(int i)
{
	switch(i)
	{
		case 0:
			return MYO_RDONLY;
		case 1:
			return MYO_WRONLY; 
		case 2:
			return MYO_RDWR;
		case 3:
			return MYO_CREAT; 
		case 4:
			return MYO_TRUNC; 
		case 5:
			return MYO_EXCL;
		case 6:
			return MYO_EXLOCK;
	}
	return -1;
}
