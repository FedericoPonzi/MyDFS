#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>    
#include <string.h>

int main()
{
	// Il file descriptor
	int fd;
	char buff[100] = "Ciao, MONDO :D";
	
	fd = open("file.txt", O_CREAT | O_RDWR);
	write(fd, buff, strlen(buff));
	
	close(fd);
	fd = open("file.txt", O_WRONLY | O_TRUNC);
	close(fd);
	
	return 0;
}
