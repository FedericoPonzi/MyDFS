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
#include <unistd.h>
#include <ctype.h>
#include "inc/OPE.h"
#include "inc/Heartbeating.h"
#include "inc/StruttureDati.h"
#include "inc/Utils.h"
#include "inc/CLOSE.h"
#include "inc/Config.h"
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
	char answer[3] = "ok";
	char ret_val[30];
    char prt_msg[PRT_MSG_SIZE+1];
    
	int err_code, controlSocket, port_num, nRecv;

	
	OpenedFile* id;

	char* nomeFile = getFileNameFromCommand(command);	 /** @todo : Da vedere bene per memory leaks!!!!*/

    int modo = getModo(command);
    	
	if((err_code = appendOpenedFile(nomeFile, modo, &id)))
	{
		logM("[appendOpenedFile] - Non posso farlo john\n");
		sprintf(ret_val, "%s", ((err_code == -3) ? "-3\n" : "-1\n"));
	}
    else
    {
        //Mando la dimensione del file
        int fileSize = 0;
        logM("filename: %s\n\n", id->fileName);
        char temp_path[40];
        strcpy(temp_path, rootPath);
        strcat(temp_path, nomeFile);
        
        if(access(temp_path, F_OK) != -1) //Se esiste mi calcolo la dimensione
        {
            fseek(id->fp,0,SEEK_END);
            fileSize = ftell(id->fp);
            fseek(id->fp,0, SEEK_SET);
        }
        id->filesize = fileSize;
		id->socketId = socket;
		sprintf(ret_val, "%d", fileSize);
    }
	
	//Mando il codice di errore se presente, e se c'e' un errore mi fermo.
	if(send(socket, ret_val, strlen(ret_val), 0) < 0 || err_code != 0)
	{
		logM("Errore nell' apertura del file (o nella send), byebye\n");
		closeClientSession(getptid());
        close(socket);
		return;
	}

	//server di nuovo in ascolto per fetch port number	
	if((nRecv = recv(socket, prt_msg, sizeof(prt_msg), 0)) < 0)
	{
		logM("[handleOpenCommand] - errore rcv port no\n");
        err_code = -2;
	}
	
	if(strncmp(prt_msg, "port_num", 8))
	{
		logM("[handleOpenCommand] - errore formato port no\n");
        err_code = -2;
	}
	else
	{
		prt_msg[nRecv] = '\0';
		port_num = strtol(prt_msg+(strlen("port_num ")), NULL, 10);
        if(errno == EINVAL || errno == ERANGE)
        {
            err_code = -2;
        }
	}
    
	//Se c'e' gia' un errore, non devo creare la socket di controllo.
	if(err_code || (controlSocket = createControlSock(port_num, socket)) < 1)
	{
        err_code = -2;
	}

    //Se e' in modalita' scrittura, spawno l' heartbeating
    if(!err_code && (isModoApertura(modo, MYO_WRONLY) || isModoApertura(modo, MYO_RDWR)))
    {
        spawnHeartBeat(id);
    }

    if(err_code)
    {
        sprintf(answer, "-2");
    }
    //Provo a mandare eventuale messaggio d' errore.
	if(send(socket, answer, strlen(answer), 0) < 0 || err_code)
    {
        closeClientSession(getptid());
        close(socket);
        return;
    }
    free(nomeFile);
    logM("[OpenCommand] Connessione creata correttamente.[\n Filename: %s,\n Modo: %d,\n Socket: %d,\n HB: %d,\n ptid: %lu.\n]", id->
    fileName, id->modo, id->socketId, id->transferSockId, getptid());
}

/**
 * @brief crea connessione dati lato server
 *
 */
int createControlSock(int portNo, int socketId)
{
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];

	len = sizeof addr;
    
	getpeername(socketId, (struct sockaddr*)&addr, &len);

	//deal with both IPv4 and IPv6:
	
	struct sockaddr_in *s = (struct sockaddr_in *)&addr;
	inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	//logM("indirizzo = %s\n", ipstr);
    
    struct sockaddr_in serv_addr;
		
	memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNo);
        
    if(inet_aton(ipstr, &serv_addr.sin_addr) == 0)
    {
		logM("error inet_aton\n");
        return 0;
    }
    
    int sd;
    
    if(( sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return 0;
    }
	
    if((connect(sd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) != 0)
    {
	   perror("connect");
	   return 0;
    }	
    OpenedFile* file = getOpenedFile(); //@todo: passarlo come parametro.
    file->transferSockId = sd;
    
    return sd;	
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
	//logM("nomeFile = '%s'\n", nomeFile);
	return nomeFile;
}

/**
 * @brief Ritorna il modo dalla stringa del comando
 * 
 */
int getModo(char* command)
{
	if(isspace(*(command+strlen(command)-2)))
	{
		return *(command+strlen(command)-1) - '0'; // magic
	}
	return strtol(command+strlen(command)-2, NULL, 10);
}
