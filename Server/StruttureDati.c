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
#include "inc/StruttureDati.h"

/**
 * @brief Aggiunge l' oggetto in input alla fine della lista linkata dei file aperti OpenedFile
 * 
 * Se non esiste la radice mainFile la crea. Altrimenti scorre tutti i nodi fino ad arrivare all' elemento in coda, crea un nuovo
 * nodo con i dati in input e lo appende alla fine.
 */
static void appendOpenedFile(char* nomefile, int modo, int socket)
{
	OpenedFile* prova;
	prova = (OpenedFile*) malloc(sizeof(OpenedFile));
	prova->filename = nomefile; 
	prova->modo = modo;
	prova->socketId = socket;
	prova->next =NULL;
	if(mainFile == NULL)
	{
		mainFile = prova;
		logM("[OpenFile] Main non esiste. Lo creo. '%s'\n", mainFile->filename);
	}
	else
	{
		mainFile->next = prova;
		mainFile = prova;
		logM("[OpenFile] Main esiste '%s' \n", mainFile->filename);
		OpenedFile* iterator = mainFile;
		while(iterator->next != NULL)
		{
			iterator = iterator->next;
		}
		iterator->next = prova;
	}
	
}

/**
 * @brief Aggiunge un nodo alla lista di File Aperti
 * 
 * Prende i dati necessari, e appende in coda a OpenedFile un nuovo elemento con i dati presi in input.
 * Se MainFile non e' stato ancora creato, lo crea.
 */
static void aggiungiOpenedFile(char* nomefile, int modo, int socket)
{

	
}


int fileAlreadyOpen(char* fileName)
{
	OpenedFile* iterator = mainFile;
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
