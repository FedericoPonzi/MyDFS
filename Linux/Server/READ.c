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
 * @file READ.c
 * @author Federico Ponzi
 * @date 11.07.2015
 * @brief Gestisce il comando READ
 *
 * Quando il client invia un comando READ, questo modulo lo gestisce, inviando sulla socket il dato richiesto.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include "inc/Utils.h"
#include "inc/StruttureDati.h"
#include "inc/READ.h"
#include "inc/Config.h"
#include <errno.h>

int getPosFromCommand(char*);

void handleREADCommand(char* command, int socket)
{
    logM("[READ]: Ricevuta richiesta read. Command = %s\n", command);

    int nRead, nSend, pos;
	
    pos = getPosFromCommand(command);

	logM("pos = %d\n", pos);

    char buff[FILESIZE];
	
	OpenedFile* id = getOpenedFile();

    //Mi sposto all' offset indicato dal client:
    fseek(id->fp, pos, SEEK_SET);
    
	nRead = fread(buff, 1, FILESIZE, id->fp);

	if (nRead < FILESIZE)
	{		
		if (ferror(id->fp))
		{
			logM("Error reading\n");
			/* Error*/
            nRead = -1;
		}
	}
    
	logM("[READ] Bytes letti: %d \n", nRead);     

    char message[15]; // strlen("size 65535") e' il massimo che mandero'.
    
    sprintf(message, "size %d", nRead);


    //Mando la dimensione della parte che ho letto
    nSend = send(socket, message, sizeof(message), 0);
    if(nSend < 0)
    {
        perror("[handleReadCommand] 1-send");
        closeClientSession(getptid());
        close(socket);
        return;
    }
    else if(nRead < 0) //diamo al client la possibilita' di riprovare.
	{
        return;
    }
    //Mando la parte letta:
    nSend = send(socket, buff, nRead, 0);
    
    if(nSend<0)
    {
        perror("[handleReadCommand] 2-send");
        closeClientSession(getptid());
        close(socket);
        return;
    }	
    else printf("Inviati al client %d dati\n", nSend);
}

/**
 * Ritorna la posizione della read richiesta dall' utente.
 */
int getPosFromCommand(char* command)
{
	int toRet;
    toRet = strtol(command+4, NULL, 10);
    if(errno == EINVAL || errno == ERANGE)
    {
        toRet = -1;
    }
    return toRet;
}
