
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int
main(int argc, char ** argv)
{
	FILE * file;
	int numr;
	char* buffer = "HELLO WORLD!";
	char buff[100];
	file = fopen("stuff.txt","w+");
	
	numr= fwrite(buffer, 1, strlen(buffer),file);
	if(numr == 0)
	{
		if(ferror(file))
			perror("fwrite");
	}
	fclose(file);
	
	file = fopen("stuff.txt", "r");
	
	numr = fread(buff, 1, sizeof(buff), file);
	
	printf("numr: %d\n %s\n	", numr, buff);
	
	fclose(file);
		
	
	return 0;
}
