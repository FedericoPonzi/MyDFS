#ifndef CACHE
#define CACHE

FILE* createTempFile(char* basename);
int readRequest(MyDFSId* id, long pos, unsigned int size, CacheRequest* req);
int writeCache(MyDFSId* id, void* buffer, int size, int pos);

#endif
