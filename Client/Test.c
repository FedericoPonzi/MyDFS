/*
 * Test.c
 * 
 * Copyright 2015 isaacisback <isaacisback@mrisaac>
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
 * Questo modulo contiene tutti i testi per tutte le funzioni da richiamare da un programma.
 */



#include <stdio.h>
#include "inc/Utils.h"
#include "inc/Config.h"
#include "inc/OPEN.h"
#include "inc/CLOSE.h"
#include "inc/READ.h"
#include "inc/WRITE.h"
#include "inc/Utils.h"
#include "assert.h"
#include <string.h>
#include "inc/Test.h"
#include <unistd.h>



/**
 * @brief Testa tutte le opzioni di apertura della open.
 * 
 */
int testOpen(char* filename)
{
	/*
	 * Modalita' delle open:
	 */	
	 int ArrayModoOpen[] = { MYO_RDONLY, MYO_WRONLY, MYO_RDWR, MYO_CREAT, MYO_TRUNC, MYO_EXCL, MYO_EXLOCK};

	printf("\t[INIZIO TEST DELLA OPEN]\n");
	int i;
	MyDFSId* fileId;
	char* indirizzo = "127.0.0.1";
	int error = 0;
	for(i = 0; i < 7; i++)
	{
		printf("\n * Modo numero: %d \n", ArrayModoOpen[i]);
		fileId = mydfs_open(indirizzo, filename, ArrayModoOpen[i], &error);
		
		if(ArrayModoOpen[i] == MYO_EXCL)
		{
			logM("MYO_EXCL percio' tutto ok c:\n");
			error = 0;
			continue;
		}
		if(error != 0)
		{
			printf("[TestOpen] Errore %d\n", error);
			assert(0);
		}
		mydfs_close(fileId);
	}
	printf("\n\t[V] Test superato correttamente!\n");
	return error;
}


int testRead(char* filename)
{
	
	
	return 0;
}


/**
 *@brief Testa l' heartbeating.
 * Visto che HB si trova su un altro thread, per comodita' non viene effettuata nessuna assert. La corretta uscita da questa
 * funzione quindi non indica il superamento del test!!
 */
int testHeartBeating(char* filename, char* indirizzo)
{
	int heartBeatingServer = 2;
	int sleepTime = 8, error = 0;
	MyDFSId* fileId;
	printf("\n\n\t[INIZIO TEST HEARTBEATING ]\n Mi aspetto %d ping.\n", sleepTime/heartBeatingServer -1);
	fileId = mydfs_open(indirizzo, filename, MYO_WRONLY, &error);
	sleep(sleepTime); // 3 ping
	mydfs_close(fileId);
	printf("Quante sono i ping? 3 = passato\n");
	return 0;
}

int testWrite(char* filename)
{
	printf("\t[TEST WRITE]\n");

	int n, error = 0, i = 0;
	char bufferPiccolo[170];
	
	char* testo = "Io sono il primo testo";
	char bufferTesto[strlen(testo)];
	MyDFSId* fileId = mydfs_open("127.0.0.1", "file.txt", MYO_RDWR, &error);
	assert(error==0);
	
	char* testoDue = "questo e' il secondo test della write.";
	
	printf("\n\t[PRIMO TEST WRITE: SEEK_SET]\n");
	
	n = mydfs_write(fileId, MYSEEK_SET, testo, strlen(testo));

	assert(n > 0);
	
	n = mydfs_read(fileId, MYSEEK_SET, bufferTesto, sizeof(bufferTesto)-1);	
	assert(n > 0);
	
	bufferTesto[n] = '\0';
	
	for(i = 0; i < n; i++)
	{
		assert(testo[i] == bufferTesto[i]);
	}
	
	int oldFileSize = fileId->filesize;
	printf("\n\t[V] Test superato correttamente!\n");
	printf("\n\t[SECONDO TEST: SEEK_END]\n");

	n = mydfs_write(fileId, MYSEEK_END, testoDue, strlen(testoDue));

	assert(n > 0);

	n = mydfs_read(fileId, MYSEEK_SET, bufferPiccolo, sizeof(bufferPiccolo)-1);	

	assert(n > 0);	
	
	bufferPiccolo[n] = '\0';
	
	for(i = 0; i < strlen(testoDue); i++)
	{
		assert(testoDue[i] == bufferPiccolo[oldFileSize]);
		oldFileSize++;
	}
	printf("\n\t[V] Test superato correttamente!\n");
	mydfs_close(fileId);	
		
	return 0;
}

/**
 * Testa l' invalidazione della cache.
 */
void testInvalidazioneCache(char* filename)
{
	printf("\t [Test di invalidazione della cache]\n");
	int error = 0, n;
	char bufferPrima[100], bufferDopo[100];
	
	char* testo = "Questo e' un test per l' invalidazione della cache.";

	MyDFSId* fileRead = mydfs_open("127.0.0.1", "file.txt", MYO_RDONLY, &error);	
	
	assert(error == 0);
	
	MyDFSId* fileWrite = mydfs_open("127.0.0.1", "file.txt", MYO_RDWR, &error);	
	
	assert(error == 0);
	
	n = mydfs_read(fileRead, MYSEEK_SET, bufferPrima, sizeof(bufferPrima)-1);

	assert( n > 0);
	
	bufferPrima[n] = '\0';
	
	printf("BufferPrima: '%s'\n", bufferPrima);
	
	n = mydfs_write(fileWrite, MYSEEK_SET, testo, strlen(testo));
	
	assert(n > 0);
	
	mydfs_close(fileWrite);
    sleep(5);
	n = mydfs_read(fileRead, MYSEEK_SET, bufferDopo, sizeof(bufferDopo)-1);
	
	assert(n > 0);

	bufferDopo[n] = '\0';
	printf("BufferDopo : '%s'\n", bufferDopo);
	mydfs_close(fileRead);
	
	assert(strcmp(bufferPrima, bufferDopo));
	
	printf("\n\t[V] Test superato correttamente!\n");
}
	
