#ifndef WRITE
#define WRITE

int mydfs_write(MyDFSId* dfsid, int pos, void *ptr, unsigned int size);
int writeTo(FILE* id, void* ptr, int size);
void addWriteOp(MyDFSId* id, int pos, int size);

#endif
