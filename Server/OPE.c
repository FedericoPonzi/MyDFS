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
 * 
 * 
 * @todo dovrebbe mandare una risposta al client in caso di successo o errore
 * @todo permettere l' apertura se il modo con cui e' aperto il file e' lettura e viene richiesta la lettura.
 */
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "inc/OPE.h"
#include "inc/Heartbeating.h"
#include "inc/StruttureDati.h"
#include "inc/Utils.h"
#include <limits.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PRT_MSG_SIZE 14

void getFileName(char* command, char* nomeFile);
int getModo(char* command);

/**
 * @brief Gestisce il comando Open.
 *
 */
void handleOpenCommand(char* command, int socket)
{
	stripCommand(command);

	char* nomeFile;
	char* ret_val;
	int err_code;
	
	nomeFile = malloc(30*sizeof(char)); /** @todo : Da vedere bene per memory leaks!!!!*/

	getFileName(command, nomeFile);

	logM("Nome del file: '%s'\n", nomeFile);
	int modo = getModo(command);
	logM("Modo di apertura: '%d'\n", modo);
	
	if((err_code = appendOpenedFile(nomeFile, modo)) != 0)
	{
		logM("[appendOpenedFile] - Non posso farlo john\n");
		ret_val = ((err_code == -3) ? "-3\n" : "-1\n");
	}
	else
	{
		ret_val = "ok";
	}
	
	if(err_code == 0 && (isModoApertura(modo, MYO_WRONLY) || isModoApertura(modo, MYO_RDWR)))
	{
		spawnHeartBeat(socket);
	}

	send(socket, ret_val, strlen(ret_val), 0);
	
	//server di nuovo in ascolto per fetch port number
	int nRecv;
	char prt_msg[PRT_MSG_SIZE];
	int port_num;
	char answer[10];
	if((nRecv = recv(socket, prt_msg, sizeof(prt_msg), 0)) < 0)
	{
		//errore
		logM("[handleOpenCommand] - errore rcv port no\n");
		strcpy(answer, "-2");
	}
	else
	{
		if(strncmp(prt_msg, "port_num", 8) == 0)
		{
			//fetch numero porta
			port_num = strtol(prt_msg+(strlen("port_num ")), NULL, 10);
			strcpy(answer, "ok");
		}
		else
		{
			//errore
			logM("[handleOpenCommand] - errore formato port no");
			strcpy(answer, "-2");
		}
	}
	
	if (createDataSock(port_num, socket))
	{
		strcpy(answer, "-2");
	}
	
	send(socket, answer, strlen(answer), 0);
}

/**
 * @brief crea connessione dati lato server
 */
int createDataSock(int portNo, int socketId)
{
	struct sockaddr_in datasock_addr; 
	socklen_t address_size;
	address_size = sizeof(datasock_addr);
	
	memset(&datasock_addr, '0', sizeof(datasock_addr)); 

    datasock_addr.sin_family = AF_INET;
    datasock_addr.sin_port = htons(portNo); 

    if(getpeername(socketId, (struct sockaddr *)&datasock_addr.sin_addr, &address_size) != 0)
    {
		perror("getpeername");
		return 1;
    } 
	
    if((connect(socketId, (struct sockaddr *)&datasock_addr, sizeof(datasock_addr))) != 0)
    {
	   perror("connect");
	   return 1;
    }	
    return 0;	
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
	logM("nomeFile = %s\n", nomeFile);
}

/**
 * @brief Ritorna il modo, sia se e' a una cifra che se e' a due cifre :D
 * 
 */
int getModo(char* command)
{
	if(isspace(*(command+strlen(command)-2)))
	{
		return *(command+strlen(command)-1) - '0'; // magic don't touch
	}
	
	char* ptr = command+ strlen(command)-2;
	long val;
	val = strtol(ptr, NULL, 10);
	return val;
}
