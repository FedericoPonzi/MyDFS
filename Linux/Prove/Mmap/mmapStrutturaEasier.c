#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LIST_SZ 128
#define WORKERS 2

static struct list_node **list_head;
static struct list_node **free_head;
static pthread_mutex_t *mutex;

typedef struct list_node {
    long value;
    struct list_node *next;
}list_node;

static struct list_node **list_head;
static struct list_node **free_head;
static pthread_mutex_t *mutex;

void print_list(void) {
    struct list_node *curr = *list_head;

    while (curr != NULL) {
        printf("%ld -> ", curr->value);
        curr = curr->next;
    }

    printf("NULL\n");
}

void do_work(void) 
{

	pthread_mutex_lock(mutex);

	struct list_node *n = *free_head;

	if (n == NULL) {
		pthread_mutex_unlock(mutex);
		assert(0);
	}

	*free_head = (*free_head)->next;
	n->value = (long) getpid();
	n->next = *list_head;
	*list_head = n;

	print_list();

	pthread_mutex_unlock(mutex);
    
}

int main(void) {
    void *ptr;
    size_t region_sz = 0;

    /* Space for the nodes */
    region_sz += sizeof(**list_head)*MAX_LIST_SZ;
	printf("Size of sizeof(**list_head)*MAX_LIST_SZ: %lu\n", region_sz);
	printf("Size del singolo: %lu\n", sizeof(**list_head));
	printf("Size of di un nodo invece e' %lu \n", sizeof(list_node));
    /* Space for house-keeping pointers */
    region_sz += sizeof(list_head)+sizeof(free_head);
	
	
	printf("sizeof(list_head) %lu, sizeof(free_head) : %lu\n", sizeof(list_head), sizeof(free_head));
	printf("Contenuto di list_head: %p\n", list_head);
    /* Space for the mutex */
    region_sz += sizeof(*mutex);
	printf("sizeof(*mutex): %lu\n", sizeof(*mutex));
	
    ptr = mmap(NULL, region_sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap(2) failed");
        exit(EXIT_FAILURE);
    }

    /* Set up everything */
    mutex = ptr;
    printf("Mutex si trova qui': %p\n", mutex);
    free_head = (struct list_node **) (((char *) ptr)+sizeof(*mutex));
    printf("freehead invece: %p\n", free_head);
    printf("freehead invece: %p\n", *free_head);

    list_head = free_head+1;

    *free_head = (struct list_node *) (list_head+1);

    *list_head = NULL;

    /* Initialize free list */
    int i;
    struct list_node *curr;

    for (i = 0, curr = *free_head; i < MAX_LIST_SZ-1; i++, curr++) {
        curr->next = curr+1;
		printf("curr->next : %p", curr->next);
		printf(" - curr invece vale: %p\n", curr);
    }

    curr->next = NULL;

    pthread_mutexattr_t mutex_attr;
    if (pthread_mutexattr_init(&mutex_attr) < 0) {
        perror("Failed to initialize mutex attributes");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) < 0) {
        perror("Failed to change mutex attributes");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(mutex, &mutex_attr) < 0) {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < WORKERS; i++) {
        pid_t pid;
        if ((pid = fork()) < 0) {
            perror("fork(2) error");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            do_work();
            return 0;
        }
    }

    for (i = 0; i < WORKERS; i++) {
        if (wait(NULL) < 0) {
            perror("wait(2) error");
        }
    }

	print_list();
    //assert(*list_head == NULL);

    return 0;
}