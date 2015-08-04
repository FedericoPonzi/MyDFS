#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>    
#include <string.h>
#include <stdlib.h>
int main()
{
	// Il file descriptor
	FILE* fd;
	char buff[100] = "Ciao, MONDO :D";
	
	fd = fopen("file.txt", "rb");
	
	fread(buff,sizeof(buff), 0, fd);
	printf("Buff: %s", buff);
		
	return 0;
}
