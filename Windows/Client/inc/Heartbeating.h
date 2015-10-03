#ifndef HEARTBEATING
#define HEARTBEATING
#include "Config.h"
void spawnHeartBeat( MyDFSId* id);
void* heartBeat(void *sd);

extern HANDLE *hbMutex;

#endif
