#include <stdio.h>
#include <string.h>





int main()
{
	char* var = "POS 1000 SIZE 20000000";
	printf("Pos: %d, Size: %d", getChunkPosition(var), getChunkSize(var));	
	
	return 0;
}
