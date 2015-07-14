#include <stdio.h>
#include "ClientIncludes.h"

int main(){
	logM("Hello, world\n");

	struct in_addr indirizzo;
	inet_aton("127.0.0.1", &indirizzo);
	int z = 23;
	char* file = "FILE.TXT";
	mydfs_open(&indirizzo, file, 23, &z);
	return 0;
}

