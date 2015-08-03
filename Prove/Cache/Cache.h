#ifndef CACHE
#define CACHE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
int writeReq(int fd, const void *buf, int count);
int readReq(int fd, void *buf, int count);
#endif
