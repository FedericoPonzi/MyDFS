#include <stdio.h>
#include "ClientIncludes.h"

int main()
{
	struct in_addr indirizzo;
	inet_aton("127.0.0.1", &indirizzo);
	char* file = "FILE.TXT";
	int err = 0;
	printf("HELLO, CLIENT \n");
	
	mydfs_open(&indirizzo, file, MYO_CREAT | MYO_RDONLY, &err);
	
	return 0;
}

