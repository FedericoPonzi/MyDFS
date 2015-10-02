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
#include <stdlib.h>
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
int testOpen(char* filename, char* indirizzo, int  debug)
{
    printf("[TEST OPEN] Iniziato \n");

	/*
	 * Modalita' delle open:
	 */	
	int ArrayModoOpen[] = { MYO_RDONLY, MYO_WRONLY, MYO_RDWR, MYO_CREAT, MYO_TRUNC, MYO_EXCL, MYO_EXLOCK};

	if(debug) printf("\t[INIZIO TEST DELLA OPEN%d]\n", getpid());
	int i;
	MyDFSId* fileId;
	int error = 0;
	for(i = 0; i < 7; i++)
	{
		if(debug) printf("\n * Modo numero: %d (pid:%d)\n", ArrayModoOpen[i], getpid());
		fileId = mydfs_open(indirizzo, filename, ArrayModoOpen[i], &error);
		
		if(ArrayModoOpen[i] == MYO_EXCL)
		{
			logM("MYO_EXCL percio' tutto ok c: %d\n", getpid());
			error = 0;
			continue;
		}
		if(error != 0)
		{
			if(debug) printf("[TestOpen-%d] Errore %d\n", getpid(), error);
			assert(0);
		}
		mydfs_close(fileId);
        sleep(0.1);
	}
	printf("\n\t[V-%d] Test OPEN superato correttamente!\n", getpid());
    return error;
}   

/**
 * SimpleRead
 */
int testSimpleRead(char* filename, char* indirizzo, int debug)
{
    printf("Inizio test SimpleRead. Apertura e chiusura.\n");
    
    int error;
    MyDFSId* id = mydfs_open(indirizzo, filename, MYO_RDWR, &error);
    assert(error == 0);
    mydfs_close(id);
    return 0;
}

/**
 * NOTA: La parte e funziona solo in locale.
 */
int testRead(char* filename, char* indirizzo, int  debug)
{
    printf("[TEST READ] Iniziato \n");
    MyDFSId* id;
    int error, n;
    char* bufferA = "Questo e' un test della read, ma scrivo pure";
    int buffsize =strlen(bufferA)+1;
    char bufferB[buffsize];
    
	if(debug) printf("\n\t [Test della READ %d]:\n", getpid());
	
	id = mydfs_open(indirizzo, filename, MYO_RDWR, &error);
    
    assert(error == 0);
    
    if(debug) printf(" * Scrivo dati nel file (mydfs_write) %d\n", getpid());

    n = mydfs_write(id, MYSEEK_CUR, bufferA, buffsize-1);

    assert(n > 0);
    mydfs_close(id);

    id = mydfs_open(indirizzo, filename, MYO_RDONLY, &error);

    assert(id != NULL);

    if(debug) printf(" * 1- Simulazione fseek(SEEK_SET%d)\n", getpid());

    n = mydfs_read(id, MYSEEK_SET, NULL, 0);

    assert(n == 0);

    if(debug) printf("\n\t[V-%d] Test 1 superato correttamente!\n", getpid());
    
    if(debug) printf("Inizio test varie modalita' di seek:\n\n");
    if(debug) printf("[2 - MYSEEK_CUR %d]\n", getpid());
	n = mydfs_read(id, MYSEEK_CUR, bufferB, buffsize-1);

    assert(n > 0);

    bufferB[n] = '\0';

    if(debug) printf("Letti %d dati, Buffer contiene: '%s'\n", n, bufferB);

    assert(!strcmp(bufferA, bufferB));

    if(debug) printf("\n\t[V-%d] Test 2 superato correttamente!\n", getpid());

    //Pulisco il buffer:
    memset(bufferB, 0, strlen(bufferB));
    
	if(debug) printf("[3 - MYSEEK_END %d]\n", getpid());
    
	n = mydfs_read(id, MYSEEK_END, bufferB, buffsize-1);
	
    assert(n==0);

    if(debug) printf("\n\t[V-%d] Test 3 superato correttamente!\n", getpid());

	if(debug) printf("[4- MYSEEK_SET]\n");
	
	n = mydfs_read(id, MYSEEK_SET, bufferB, buffsize-1);	

    assert(n > 0);

    if(debug) printf("\n\t[V] Test 4 superato correttamente!\n");
	if(debug) printf(" * Seek to 0\n");
	n = mydfs_read(id,MYSEEK_SET, NULL, 0);

    assert(n == 0);
    if(debug) printf("[5 - Inizio test di read piu lunga]\n");

    char megabuffer[id->filesize+1];
    char bufferC[50];
    
    int i = 0;
	while((n = mydfs_read(id, MYSEEK_CUR, bufferC, sizeof(bufferC)-1))> 0)
	{
        bufferC[n] = '\0';
        strcpy(megabuffer+i, bufferC);
		i+=n;
	}

    assert(n==0); //niente errori.
    
    char localbuff[sizeof(megabuffer)];
    char filepath[strlen(id->filename)+strlen("tmp")];
    sprintf(filepath, "/tmp/%s", id->filename);
    
    FILE* fp = fopen(filepath, "r");
    i = 0;
    while((n = fread(bufferC, 1, sizeof(bufferC)-1, fp)) > 0)
	{
        bufferC[n] = '\0';
        strcpy(localbuff+i, bufferC);
		i+=n;
	}
	assert(n==0);
    assert(!strcmp(megabuffer, localbuff));
    mydfs_close(id);
    if(debug) printf("\n\t[V-%d] Test 5 superato correttamente!\n", getpid());
    
    printf("\n\t[V-%d] Test READ superato correttamente.\n", getpid());

	return 0;
}


/**
 *@brief Testa l' heartbeating.
 * Visto che HB si trova su un altro thread, per comodita' non viene effettuata nessuna assert. La corretta uscita da questa
 * funzione quindi non indica il superamento del test!!
 */
int testHeartBeating(char* filename, char* indirizzo, int  debug)
{
	int heartBeatingServer = 2;
	int sleepTime = 8, error = 0;
	MyDFSId* fileId;
	if(debug) printf("\n\n\t[INIZIO TEST HEARTBEATING%d]\n Mi aspetto %d ping.\n",getpid(), sleepTime/heartBeatingServer -1);
	fileId = mydfs_open(indirizzo, filename, MYO_WRONLY, &error);
	sleep(sleepTime); // 3 ping
	mydfs_close(fileId);
	printf("Quante sono i ping? 3 = passato\n");
	return 0;
}

/**
 * @brief Testa la write
 * NOTA: bufferpiccolo deve essere maggiore o uguale alla dimensione di file.txt altrimenti il test non verra' superato.
 */
int testWrite(char* filename, char* indirizzo, int  debug)
{
    printf("[TEST WRITE] Iniziato \n");

    int pid = getpid();
	if(debug) printf("\t[TEST WRITE-%d]\n", pid);

	int n, error = 0, i = 0;
	char bufferPiccolo[1024];
	
	char* testo = "Primo";
	char bufferTesto[strlen(testo)+1];
	MyDFSId* fileId = mydfs_open(indirizzo, filename, MYO_RDWR, &error);
	assert(error==0);
	
	char* testoDue = "Secondo";
	
	if(debug) printf("\n\t[PRIMO TEST WRITE-%d: SEEK_SET]\n", getpid());
	
	n = mydfs_write(fileId, MYSEEK_SET, testo, strlen(testo));

	assert(n > 0);
	
	n = mydfs_read(fileId, MYSEEK_SET, bufferTesto, sizeof(bufferTesto)-1);	
	assert(n > 0);
	
	bufferTesto[n] = '\0';
	
	for(i = 0; i < n; i++)
	{
		assert(testo[i] == bufferTesto[i]);
	}
	
	if(debug) printf("\n\t[V-%d] Test superato correttamente!\n", getpid());

	if(debug) printf("\n\t[SECONDO TEST-%d: SEEK_END]\n", getpid());

	int oldFileSize = fileId->filesize;
	n = mydfs_write(fileId, MYSEEK_END, testoDue, strlen(testoDue));

	assert(n > 0);

	n = mydfs_read(fileId, MYSEEK_SET, bufferPiccolo, sizeof(bufferPiccolo)-1);	

	assert(n > 0);	
	
	bufferPiccolo[n] = '\0';
    if(debug) printf("Letto: %s", bufferPiccolo);
	mydfs_close(fileId);
    for(i = 0; i < strlen(testoDue); i++)
	{
        if(!testoDue[i] == bufferPiccolo[oldFileSize])
        {
            printf("Mi aspetto: %s, ho trovato :%s, sono: %d\n", testoDue, bufferPiccolo, getpid());
        }
		assert(testoDue[i] == bufferPiccolo[oldFileSize]);
		oldFileSize++;
	}
	printf("\n\t[V-%d] Test WRITE superato correttamente!\n", getpid());

		
	return 0;
}

/**
 * Testa l' invalidazione della cache.
 */
void testInvalidazioneCache(char* filename, char* indirizzo, int  debug)
{
    printf("[TEST INVALIDAZIONECACHE] Iniziato \n");

	if(debug) printf("\t [Test di invalidazione della cache]\n");
	int error = 0, n;
	char bufferPrima[100], bufferDopo[100];
	
	char* testo = "Questo e' un test per l' invalidazione della cache.";

	MyDFSId* fileRead = mydfs_open(indirizzo, "file.txt", MYO_RDONLY, &error);	
	
	assert(error == 0);
	
	MyDFSId* fileWrite = mydfs_open(indirizzo, "file.txt", MYO_RDWR, &error);	
	
	assert(error == 0);
	
	n = mydfs_read(fileRead, MYSEEK_SET, bufferPrima, sizeof(bufferPrima)-1);

	assert( n > 0);
	
	bufferPrima[n] = '\0';
	

	
	n = mydfs_write(fileWrite, MYSEEK_SET, testo, strlen(testo));
	
	assert(n > 0);
	
	mydfs_close(fileWrite);
    sleep(4);
	n = mydfs_read(fileRead, MYSEEK_SET, bufferDopo, sizeof(bufferDopo)-1);
	
	assert(n > 0);

	bufferDopo[n] = '\0';

	mydfs_close(fileRead);
	
	assert(!strcmp(bufferPrima, bufferDopo));
	
	printf("\n\t[V-%d] Test INVALIDAZIONECACHE superato correttamente!\n", getpid());
    }

/* Returns an integer in the range [0, n).
 *
 * Uses rand(), and so is affected-by/affects the same seed.
 */
int randint(int n) {
  if ((n - 1) == RAND_MAX) {
    return rand();
  } else {
    // Chop off all of the values that would cause skew...
    long end = RAND_MAX / n; // truncate skew
    assert (end > 0L);
    end *= n;

    // ... and ignore results from rand() that fall above that limit.
    // (Worst case the loop condition should succeed 50% of the time,
    // so we can expect to bail out of this loop pretty quickly.)
    int r;
    while ((r = rand()) >= end);

    return r % n;
  }
}

/**
 * Sceglie un test a caso e lo esegue
 */
void testStressTest(char* filename, char* indirizzo, int debug)
{
    
	if(debug) printf("\n\t [Test stress%d:]\n", getpid());
    int test = randint(8);
    printf("test vale:%d\n", test);
	sleep(randint(2));
    if(test > 8)
    {
        testRead(filename, indirizzo, debug);
    }
    else if(test > 4)
    {
        testWrite(filename, indirizzo, debug);
    }
    else if(test > 2)
    {
        testInvalidazioneCache(filename, indirizzo, debug);
    }
    /*else if(test > 2)
    {
        testLongHold(filename, indirizzo, debug);
    }*/
    else
    {
        testOpen(filename, indirizzo, debug);
    }
}
