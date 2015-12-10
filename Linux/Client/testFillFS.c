#include <stdio.h>
#include <string.h>
#include "inc/Utils.h"
#include "inc/Config.h"
#include "inc/OPEN.h"
#include "inc/CLOSE.h"
#include "inc/READ.h"
#include "inc/WRITE.h"
#include "inc/Utils.h"
#include <assert.h>

char * files[] = { "file1.txt", "file2.txt", "fileLOl.mp3", "mioFile.txt"};

int main()
{
    MyDFSId* id;
    int i;
    char buffer[150];
    int err;
    int ret = 0;
    for(i = 0; i < 4; i++)
    {
        id = mydfs_open("127.0.0.1", files[i], MYO_CREAT | MYO_WRONLY, &err);
        assert(err == 0);
        sprintf(buffer, "CIAO MONDO :D Sono il file: '%s'", files[i]);
        ret = mydfs_write(id, MYSEEK_SET, buffer, strlen(buffer));
        assert(ret > 0);
        mydfs_close(id);
    }

    return 0;
}
