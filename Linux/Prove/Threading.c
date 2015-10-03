#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

pthread_t tid[2];

void* doSomeThing(void *arg)
{
    int i = 0;
    pthread_t id = pthread_self();

    if(pthread_equal(id,tid[0]))
    {
		i = 20;
        printf("\n First thread processing\n");
    }
    else
    {
		i = 11;
        printf("\n Second thread processing\n");
    }
	printf("i si trova qui': %p\n", &i);
	printf("i vale: %d\n", i);
    return NULL;
}

int main(void)
{
    int i = 0;
    int err;

    while(i < 2)
    {
        err = pthread_create(&(tid[i]), NULL, &doSomeThing, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");

        i++;
    }

    sleep(5);
    return 0;
}
