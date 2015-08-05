#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

FILE* createTempFile(char* basename);

int main(void)
{
	FILE* myTempFile = createTempFile("CiaoMondo");
	char* buffer = "CIAO MONDO :D";
	fwrite(buffer, strlen(buffer), 1, myTempFile); 
	
	char buffert[strlen(buffer)+1];
	fseek(myTempFile, 0, SEEK_SET);
	int i = fread(buffert, strlen(buffer), 1, myTempFile);
	buffert[sizeof(buffert)]='\0';
	printf("\nHo letto dal file: '%s'\n", buffert);
    return 0;
}

FILE* createTempFile(char* basename)
{
	char tempName[strlen(basename) + 8];
	int fd;
	FILE* toRet;
	sprintf(tempName, "%s-XXXXXX",basename);
	//Mi creo il file temporaneo:
    fd = mkstemp(tempName);
	//Appena mi stacco dall' fd, viene cancellato il file temporaneo:
    unlink(tempName);
	printf("Creato temp file: '%s'", tempName);
	toRet = fdopen(fd, "w+");
	return toRet;
}
