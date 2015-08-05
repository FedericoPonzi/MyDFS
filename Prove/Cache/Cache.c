#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

int main(void)
{
    // buffer to hold the temporary file name
    char nameBuff[32];
    // buffer to hold data to be written/read to/from temporary file
    char buffer[24];
    int filedes = -1,count=0;

    // memset the buffers to 0
    memset(nameBuff,0,sizeof(nameBuff));
    memset(buffer,0,sizeof(buffer));

    // Copy the relevant information in the buffers
    strncpy(nameBuff,"/tmp/myTmpFile-XXXXXX",21);
    strncpy(buffer,"Hello World",11);

    errno = 0;
    // Create the temporary file, this function will replace the 'X's
    filedes = mkstemp(nameBuff);

    // Call unlink so that whenever the file is closed or the program exits
    // the temporary file is deleted
    unlink(nameBuff);

    if(filedes<1)
    {
        printf("\n Creation of temp file failed with error [%s]\n",strerror(errno));
        return 1;
    }
    else
    {
        printf("\n Temporary file [%s] created\n", nameBuff);
    }

    errno = 0;
    // Write some data to the temporary file
    if(-1 == write(filedes,buffer,sizeof(buffer)))
    {
        printf("\n write failed with error [%s]\n",strerror(errno));
        return 1;
    }

    printf("\n Data written to temporary file is [%s]\n",buffer);

    // reset the buffer as it will be used in read operation now
    memset(buffer,0,sizeof(buffer));

    errno = 0;
    // rewind the stream pointer to the start of temporary file
    if(-1 == lseek(filedes,0,SEEK_SET))
    {
        printf("\n lseek failed with error [%s]\n",strerror(errno));
        return 1;
    }

    errno=0;
    // read the data from temporary file
    if( (count =read(filedes,buffer,11)) < 11 )
    {
        printf("\n read failed with error [%s]\n",strerror(errno));
        return 1;
    }

    // Show whatever is read
    printf("\n Data read back from temporary file is [%s]\n",buffer);

    return 0;
}
