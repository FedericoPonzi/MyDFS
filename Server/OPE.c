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
#include <arpa/inet.h>

#define PRT_MSG_SIZE 14

static char* getFileNameFromCommand(char* command);
int getModo(char* command);

/**
 * @brief Gestisce il comando Open.
 *
 */
void handleOpenCommand(char* command, int socket)
{
	stripCommand(command);

	char* ret_val;
	int err_code;
	
	char* nomeFile = getFileNameFromCommand(command);	 /** @todo : Da vedere bene per memory leaks!!!!*/

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
	if(err_code != 0) return;
	
	//server di nuovo in ascolto per fetch port number
	int nRecv;
	char prt_msg[PRT_MSG_SIZE+1];
	int port_num;
	char answer[3] = "ok";
	if((nRecv = recv(socket, prt_msg, sizeof(prt_msg), 0)) < 0)
	{
		//errore
		logM("[handleOpenCommand] - errore rcv port no\n");
		strcpy(answer, "-2");
	}
	if(strncmp(prt_msg, "port_num", 8) == 0)
	{
		prt_msg[nRecv] = '\0';
		//fetch numero porta
		port_num = strtol(prt_msg+(strlen("port_num ")), NULL, 10);
	}
	else
	{
		//errore
		logM("[handleOpenCommand] - errore formato port no");
		strcpy(answer, "-2");
	}
	logM("Il client ha richiesto la connessione sulla port: %d\n", port_num);
	
	if (createDataSock(port_num, socket))
	{
		strcpy(answer, "-2");
	}
	
	send(socket, answer, sizeof(answer), 0);
}

/**
 * @brief crea connessione dati lato server
 *
 * @todo correggere utilizzo vecchio socketDes
 */
int createDataSock(int portNo, int socketId)
{
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];

	len = sizeof addr;
	getpeername(socketId, (struct sockaddr*)&addr, &len);

	//deal with both IPv4 and IPv6:
	
	struct sockaddr_in *s = (struct sockaddr_in *)&addr;
	inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	logM("indirizzo = %s\n", ipstr);
    
    struct sockaddr_in serv_addr;
		
	memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNo);
        
    if(inet_aton(ipstr, &serv_addr.sin_addr) == 0)
    {
		logM("error inet_aton\n");
        return 1;
    }
    
    int sd;
    
    if(( sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return 1;
    }
	
    if((connect(sd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) != 0)
    {
	   perror("connect");
	   return 1;
    }	
    OpenedFile* file = getOpenedFile();
    file->transferSockId = sd;
    
    return 0;	
}

/**
 * @brief Torna il nome (o path del file richiesto
 * 
 * Mi calcolo dalla fine il primo spazio, e uso quello come delimitatore come nome del file.
 */
char* getFileNameFromCommand(char* command)
{
	char* nomeFile = malloc(30*sizeof(char));
	int lunghezza = strlen(command);
	int i = lunghezza;
	while(!isspace(command[i]))
	{
		i--;
	}
	memcpy(nomeFile, command, lunghezza);
	nomeFile[i] = '\0';
	logM("nomeFile = %s\n", nomeFile);
	return nomeFile;
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
	return strtol(command+strlen(command)-2, NULL, 10);
}
