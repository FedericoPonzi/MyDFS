#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static struct list_node **list_head;
static struct list_node **free_head;
static pthread_mutex_t *mutex;


struct Nodo
{
	int val;
	struct Nodo *next;
}
struct Nodo **ll;

void addNode(int i)
{
	Nodo* iterator = *ll;
	while(iterator->next != NULL)
	{
		iterator = iterator->next;
}


int main()
{	
	
	return EXIT_SUCCESS;
}
