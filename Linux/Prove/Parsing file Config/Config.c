#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#define NUMCON "NUMBER_OF_CONNECTION"
#define PROCORTRE "PROCESS_OR_THREAD"

int numeroCon = 0;
int procortre = 0;

void handleLine(char* line)
{
	int i = 0;
	if(strlen(line) == 0)
	{ 
		return;
	}
	while(line[i] != '\0')
	{
		if(line[i] == '#' && line[i+1] == '#')
		{
			printf("Trovato commento :)\n");
			break;
		}
		else
		{
			if(strncmp(line, NUMCON, strlen(NUMCON)) == 0)
			{
				printf("Trovato numerocon\n");
				i = strlen(NUMCON) + 1; //Lo spazio;
				char * ptr;
				numeroCon = strtol(&line[i], &ptr, 10);
				break;
			}
			else if(strncmp(line, PROCORTRE, strlen(PROCORTRE)) == 0)
			{
				printf("Trovato procotre\n");
				i = strlen(PROCORTRE) +1;
				char * ptr;
				procortre = strtol(&line[i], &ptr, 10);
				break;
			}
		}
		i++;
	}
	
}

void loadConfig()
{
	char* filename = "CONFIG";
	FILE* filePointer = fopen(filename, "ru");
	char buf[256];
	while (fgets (buf, sizeof(buf), filePointer)) 
	{	
		handleLine(buf);	
		printf("%s", buf);
	}
	

}

int main()
{
	loadConfig();
	printf("Allora, ho trovato: %d numero di connessioni e %d procotre\n", numeroCon, procortre);
	return EXIT_SUCCESS;
}
