#ifndef HEARTBEATING
#define HEARTBEATING
#include "Config.h"
void spawnHeartBeat( MyDFSId* id);
void* heartBeat(void *sd);

extern pthread_mutex_t *hbMutex;

typedef struct {
    //Or whatever information that you need
    int temp_sd;
    int ptid;
} pthreadArgs;
#endif
