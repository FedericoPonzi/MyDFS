#ifndef OPEN
#define OPEN

#define MYO_RDONLY 1
#define MYO_WRONLY 2
#define MYO_RDWR   4
#define MYO_CREAT  8
#define MYO_TRUNC  16
#define MYO_EXCL   32
#define MYO_EXLOCK 64


MyDFSId* mydfs_open(struct in_addr *indirizzo, char* file, int modo, int* err);


#endif
