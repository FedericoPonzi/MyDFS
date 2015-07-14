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
#include "inc/Utils.h"

#define BUF_SIZE 256

int handleREADCommand(char* command, int socket)
{
	stripCommand(command);
	logM("[GET] Richiesto il file:'%s'", command);
	FILE *fp = fopen(command,"rb");

	if(fp==NULL)
	{
		logM("Errore nella apertura del file.\n");
		//TODO: Dovrebbe tornare al client un codice di errore.
		return -1;
	}   
	logM("[Get] File aperto correttamente. \n");
	while(1)
	{
		/* First read file in chunks of 256 bytes */
		unsigned char buff[BUF_SIZE]={0};
		int nread = fread(buff,1,BUF_SIZE,fp);
		logM("Bytes read %d \n", nread);        

		/* If read was success, send data. */
		if(nread > 0)
		{
			printf("Sending \n");
			write(socket, buff, nread);
		}

		/*
		 * There is something tricky going on with read .. 
		 * Either there was error, or we reached end of file.
		 */
		if (nread < 256)
		{
			if (feof(fp))
				printf("End of file\n");
			if (ferror(fp))
				printf("Error reading\n");
			break;
		}


	}
	return 0;
}

