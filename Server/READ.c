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

#define BUF_SIZE 65000
int getPosFromCommand(char*);

void handleREADCommand(char* command, int socket)
{
	logM("[READ]: Ricevuta richiesta read. Command = %s\n", command);
	int pos = getPosFromCommand(command);
	logM("pos = %d\n", pos);
	char* fileName = getFileName();
	char filePath[strlen(fileName) + strlen(rootPath)+1];
	sprintf(filePath, "%s%s", rootPath,fileName);
	logM("[READ] Provo ad aprire: '%s'\n", filePath);

	char buff[BUF_SIZE];
	
	OpenedFile* of = getOpenedFile();
	
	logM("[READ] File aperto correttamente. \n");
	
	//fseek(of->fp, 0, SEEK_CUR);
/*
	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	lSize = ftell (pFile);
	rewind (pFile);

	// allocate memory to contain the whole file:
	buffer = (char*) malloc (sizeof(char)*lSize);
	if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

	// copy the file into the buffer:
	result = fread (buffer,1,lSize,pFile);
	if (result != lSize) {fputs ("Reading error",stderr); exit (3);}
*/

	int nread = fread(buff,BUF_SIZE,1,of->fp);
	/*
	 * There is something tricky going on with read .. 
	 * Either there was error, or we reached end of file.
	 */
	if (nread < BUF_SIZE)
	{
		if (feof(of->fp))
		{
			nread = strlen(buff);
		}
		if (ferror(getOpenedFile()->fp))
		{
			logM("Error reading\n");
			/* Error*/
		}
	}
	logM("[READ] Bytes letti: %d \n", nread);        
	logM("[READ] Contenuto letto: '%s'\n", buff);

	if(nread > 0)
	{
		logM("[READ] Invio file...\n");
		char message[20];
		sprintf(message, "size %d", nread);
		
		//Mando la dimensione della parte che ho letto
		write(socket, message, strlen(message)+1);
		
		//Mando la parte letta
		logM("Transfer socket: '%d'\n", getTransferSocket());
		write(socket, buff, nread);
	}
	

}

int getPosFromCommand(char* command)
{
	return strtol(command+4, NULL, 10);
}
