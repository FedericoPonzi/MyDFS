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
		filename = "file.txt";
	}
	else
	{
		filename=argv[1];
	}
	char* indirizzo = "127.0.0.1";
	//testSimpleRead(filename, indirizzo, 1 );
	testRead(filename, indirizzo, 1 );
	
	return 0;	
}
