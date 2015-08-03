#ifndef HEARTBEATING
#define HEARTBEATING

void spawnHeartBeat(int sd);
void* heartBeat(void *pt_pthreadarg);

extern pthread_mutex_t *hbMutex;

typedef struct {
    //Or whatever information that you need
    int temp_sd;
    int ptid;
} pthreadArgs;
#endif
