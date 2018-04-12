#include <stdio.h>
#include <stdlib.h>
#include "inc/Utils.h"
#include "inc/Config.h"
#include "inc/OPEN.h"
#include "inc/CLOSE.h"
#include "inc/READ.h"
#include "inc/WRITE.h"
#include "inc/Utils.h"
#include "assert.h"
#include <string.h>

int main()
{
    printf("Test copia un file binario. \n");
	printf("It will use the file 'binario.tar.gz'.");
    char* filepath ="binario.tar.gz";
    int err = 0;
    int i = 0;
    void *buffer = malloc(60000);
    MyDFSId *id;
    id = mydfs_open("127.0.0.1", filepath, MYO_RDONLY, &err);
    FILE* fp = fopen("binario.tar.gz", "w+b");
    

    while((i = mydfs_read(id, MYSEEK_CUR, buffer, 60000)) > 0)
    {
        fwrite(buffer, 1, i, fp);
    }

    mydfs_close(id);
    fclose(fp);
    free(buffer);
    return 0;
}  
