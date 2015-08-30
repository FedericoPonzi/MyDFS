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

int main()
{
	int nW, nR,error = 0;
	char* indirizzo = "127.0.0.1";
	
	MyDFSId* fileId = mydfs_open(indirizzo, "file.txt", MYO_RDWR, &error);
	char bufferPiccolo[50];
	
	
	printf("\t\t\n[TEST WRITE!!11]\n");
	char* testo = "CIAO MONDO! :D";
	char* testoDue = "questo e' il secondo test della write.";
	
	printf("\n\n\t[PRIMO TEST WRITE: SEEK_SET]\n");
	
	nW = mydfs_write(fileId, MYSEEK_SET, testo, strlen(testo));
	
	assert(nW >= 0);	
	
	nR = mydfs_read(fileId, MYSEEK_SET, bufferPiccolo, sizeof(bufferPiccolo)-1);	
	
	bufferPiccolo[nR] = '\0';
	
	printf("Letti %d dati, Buffer contiene: '%s'\n", nR, bufferPiccolo);
	
	printf("\n\n\t[SECONDO TEST: SEEK_END]\n");
	nW = mydfs_write(fileId, MYSEEK_END, testoDue, strlen(testoDue));
	
	assert(nW >= 0);
	
	nR = mydfs_read(fileId, MYSEEK_CUR, bufferPiccolo, sizeof(bufferPiccolo)-1);	
	
	bufferPiccolo[nR] = '\0';
	
	printf("Letti %d dati, Buffer contiene: '%s'\n", nR, bufferPiccolo);
	
	
	mydfs_close(fileId);
	
	
	
	return 0;
}
