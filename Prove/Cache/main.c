#include "Cache.h"




int main()
{
	int i, fd;
	char buff[1000];
	fd = open("file.txt", O_CREAT | O_RDONLY);
	while ((i = readReq(fd, buff, sizeof(buff)-1)))
	{
		buff[i+1] = '\0';
		printf("\n Buffer contiene: '%s'", buff);
		printf("Letti %d dati;  ", i);
	}

	return 0;
}
