#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inc/Cache.h"


FILE* createTempFile(char* basename)
{
	char tempName[strlen(basename) + 8];
	int fd;
	FILE* toRet;
	sprintf(tempName, ".%s-XXXXXX",basename);
	//Mi creo il file temporaneo:
    fd = mkstemp(tempName);
	//Appena mi stacco dall' fd, viene cancellato il file temporaneo:
    unlink(tempName);
	printf("[Cache] Creato temp file: '%s'\n", tempName);
	toRet = fdopen(fd, "w+");
	return toRet;
}
