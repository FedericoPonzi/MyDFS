#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct Struttura
{
    int prova;
    struct Struttura* next;
} Struttura;

Struttura* glob;
int main()
{

    int fd = open("/dev/zero", O_RDWR, 0);
    glob = mmap(NULL, sizeof(Struttura), PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);

	glob->prova=2;
	
    int pid = fork();	
    if(pid == 0)
    {
        printf("Glob value: %d\n", glob->prova);
        glob->next = mmap(NULL, sizeof(Struttura), PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
        glob->next->prova = 3;
        printf("The next value: %d\n\n", glob->next->prova);
        return (0);
    }
    else
    {
		wait(NULL);
		printf("Glob value: %d\n", glob->prova);
		printf("Final The next value: %d\n\n", glob->next->prova); //Segmentation fault!
	}
    return (0);

}
