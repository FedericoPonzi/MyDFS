/*
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
 *  - Creazione del file, aggiunta dei metodi di base, aggiunta di un po di doc.
 */
 
/**
 * @file FileHandle.c
 * @author Federico Ponzi
 * @date 11.07.2015
 * @brief Contiene alcune funzioni di base per la gestione dei file.
 *
 * Questo modulo non gestisce il caching. 
 * Quello che fa e' da interfaccia per le funzioni di apertura file, chiusura, lettura e scrittura.
 */
#include <stdio.h>
#include "inc/FileHandle.h"
#include "inc/Utils.h"

/**
 * name: mydfs_open
 * 
 * @param indirizzo: l' indirizzo ipv4 a cui vogliamo connetterci, 
 * @param nomefile: nome del file richiesto, 
 * @param modo: modo, che puo essere:  O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_TRUNC, O_EXCL,  O_EXLOCK 
 * @return NULL in caso di errore, un puntatore a MyDFSId* altrimenti.
 * 
 */

MyDFSId* mydfs_open(struct in_addr* indirizzo, char *nomefile, int modo, int *err){
	logM("L'utente ha richiesto l' indirizzo: %lu", indirizzo->s_addr);
}

/**
 * name: mydfs_open
 * 
 * @param dfsid: L' id del file che l' utente vuole chiudere.
 * @return -1 in caso di errore, 0 altrimenti.
 * 
 */
int mydfs_close(MyDFSId* dfsid)
{
		logM("L'utente ha richiesto la chiusura del file ' %s'.", dfsid->filename
}

/**
 * name: mydfs_read
 * 
 * @param dfsid: L' id del file che l' utente vuole chiudere.
 * @return -1 in caso di errore, 0 altrimenti.
 * 
 */
int mydfs_read(MyDFSId dfsid, int pos, void *ptr, unsigned int size)
{
	logM("L'utente ha richiesto la lettura del file: '%s'.", dfsid->filename);
}


