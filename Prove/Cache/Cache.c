#include "Cache.h"

int readReq(int fd,void *buf, int count)
{
	return read(fd, buf, count);	
}

int writeReq(int fd, const void *buf, int count)
{
	return 0;
}

