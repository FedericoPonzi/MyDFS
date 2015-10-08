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
	char* filename;
	
	if(argc < 2)
	{
		filename = "prova/file.txt";
	}
	else
	{
		filename=argv[1];
	}
    char* indirizzo = "127.0.0.1";
    testOpen(filename, indirizzo, 0 );
    testInvalidazioneCache(filename, indirizzo, 0 );
    testRead(filename, indirizzo, 0 );
    testWrite(filename, indirizzo, 0 );
	testHeartBeating(filename, indirizzo, 0);
    return 0;	
}
