#ifndef READ
#define READ
#define MYSEEK_SET 0
#define MYSEEK_CUR 1
#define MYSEEK_END 2

int mydfs_read(MyDFSId* dfsid, int pos, void *ptr, unsigned int size);

#endif
