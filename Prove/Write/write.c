#include <stdio.h>
#include <string.h>


int main()
{
	FILE *input = fopen( "stuff.txt", "r+" );
	fseek(input, 0, SEEK_END);
	char* buffer = "rLOOL";
	fwrite(buffer, 1, sizeof(buffer)-1, input);

	fclose(input);
	return 0;
}
