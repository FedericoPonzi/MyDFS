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

int main(int argc, char* argv[])
{
	int n;
	char* indirizzo;
	int error, modo;
	MyDFSId* fileId;
	char* filename;
	filename = "file.txt";
	
	printf("FILENAME: %s\n", filename);
	
	char* fileInesistente;
	int array[] = { MYO_RDONLY, MYO_WRONLY, MYO_RDWR, MYO_CREAT, MYO_TRUNC, MYO_EXCL, MYO_EXLOCK};
	indirizzo = "127.0.0.1";

	if(argc == 2)
	{
		filename = argv[1];
	}

	error = 0;

	printf("\n\tHELLO, CLIENT \n");
	printf("[TEST DELLA OPEN]:\n");

	int z = 0;
	int i;
	for(i = 7; i < 7; i++)
	{
		int k;
		printf("\n * Modo numero: %d \n", array[i]);
		fileId = mydfs_open(indirizzo, filename, array[i], &error);
		z++;
		printf("z: %d\n", z);


		if(array[i] == MYO_EXCL)
		{
			logM("MYO_EXCL percio' tutto ok c:\n");
			continue;
		}
		if(error != 0)
		{
			printf("Errore %d\n", error);
			assert(0);
		}
		mydfs_close(fileId);
	}
	printf("[/ FINE TEST BASE DELLA OPEN]\n\n");
	printf("\t[INIZIO PROVA READ]\n");
	
	fileId = mydfs_open(indirizzo, filename, MYO_RDONLY, &error);
	if(error != 0)
	{
		printf("Errore nella open!\n");
		return 0;
	}
	char buffer[1000];
	n = mydfs_read(fileId, MYSEEK_SET, buffer, sizeof(buffer)-1);
	if(ferror(fileId->fp))
	{
		printf("Errore nella read");
	}
	//Visto che e' un file txt, devo aggiungere un fine linea.
	buffer[n]= '\0';
	
	printf("Letti %d dati, Buffer contiene: '%s'\n", n, buffer);
	mydfs_close(fileId);

	printf("\n\t [2] Test delle opzioni fseek:\n");
	
	fileId = mydfs_open(indirizzo, filename, MYO_RDWR, &error);

	if(error != 0)
	{
		printf("Errore nella open!\n");
		return 0;
	}
	char bufferPiccolo[50];
	printf("MYSEEK_CUR:\n");
	n = mydfs_read(fileId, MYSEEK_CUR, bufferPiccolo, sizeof(bufferPiccolo)-1);
	bufferPiccolo[n] = '\0';
	printf("Letti %d dati, Buffer contiene: '%s'\n", n, bufferPiccolo);
	printf("MYSEEK_END:\n");
	n = mydfs_read(fileId, MYSEEK_END, bufferPiccolo, sizeof(bufferPiccolo)-1);
	printf("N dovrebbe essere zero: %d\n", n);
	assert(n==0);
	printf("MYSEEK_SET\n");
	
	n = mydfs_read(fileId, MYSEEK_SET, bufferPiccolo, sizeof(bufferPiccolo)-1);	
	printf("Letti %d dati, Buffer contiene: '%s'\n", n, bufferPiccolo);

	printf("\n\n\t [2] Altro test della read (con piu letture):\n");
	
	printf("Seek to 0\n");
	mydfs_read(fileId,MYSEEK_SET, NULL, 0);
	printf("Mi copio %s dentro a ./%s:\n", filename, filename);
	/*FILE* mioFile = fopen(filename, "w+");
	per qualche motivo questa fopen non crea il file :/
	while((n = mydfs_read(fileId, MYSEEK_CUR, bufferPiccolo, sizeof(bufferPiccolo)-1))> 0)
	{
		//Visto che e' una stringa, devo aggiungere un fine linea.
		bufferPiccolo[n]= '\0';
		printf("Letti %d dati, Buffer contiene: '%s'\n", n, bufferPiccolo);
		fwrite(bufferPiccolo, 1, n, mioFile);
		//break;
	}
	printf("\nReads finita, ecco il file:\n");
	fseek(mioFile, 0, SEEK_SET);
	while((n = fread(buffer, 1, sizeof(bufferPiccolo)-1, mioFile))>0)
	{
		bufferPiccolo[n] = '\0';
		printf("%s", bufferPiccolo);
	}
	
	if(n<0)
	{
		printf("errore. \n");
	}*/
	
	printf("\t\t\n[TEST WRITE!!11]\n");
	char* testo = "CIAO MONDO! :D";
	
	n = mydfs_write(fileId, MYSEEK_SET, testo, strlen(testo));
	
	assert(n>=0);
	
	
	n = mydfs_read(fileId, MYSEEK_SET, bufferPiccolo, sizeof(bufferPiccolo)-1);	
	printf("Letti %d dati, Buffer contiene: '%s'\n", n, bufferPiccolo);
	
	mydfs_close(fileId);
	
	
	
	printf("\n[/ FINE TEST READ!]");
	int sleepTime = 8;
	printf("\n\n\t[INIZIO TEST HEARTBEATING ]\n Mi aspetto %d ping.\n", 8/2 -1);
	fileId = mydfs_open(indirizzo, filename, MYO_WRONLY, &error);
	sleep(8); // 3 ping
	mydfs_close(fileId);
	
	
	return 0;
}
