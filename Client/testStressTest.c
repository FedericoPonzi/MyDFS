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

int main(int argc, char* argv[])
{
    printf("Usage: TestStressTest filename.txt <numclients>\n\n");
	char* filename;
    int clients;
    if(argc < 3)
    {
        clients = 3;
    }
    else
    {
        clients=strtol(argv[2], NULL, 10);
    }
	if(argc < 2)
	{
		filename = "file.txt";
	}
	else
	{
		filename=argv[1];
	}
    char* indirizzo = "127.0.0.1";
	printf("Benvenuto allo stresstest!\nVerranno spawnati %d clients che tenteranno l'apertura e chiusura in tutte le combinazioni del file '%s'. Assicurati che il client possa servire un numero sufficente di clients prima di iniziare il test.\n\n\n", clients, filename);

    int i = 0;
    int pid;
    for(i = 0; i < clients; i++)
    {
        pid = fork();
        if(!pid)
        {
            testStressTest(filename, indirizzo);
        }
    }
    printf("processi spawnati.");
	return 0;	
}
