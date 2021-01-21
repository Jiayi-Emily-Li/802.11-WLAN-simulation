#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

typedef struct node 
{
    void* data;
    struct node* next;
}node;

struct queue 
{
    int size;
    node* head;
    node* tail;
};

queue_t queue_create()
{
    queue_t myQ;
    myQ = (struct queue*) malloc(sizeof(struct queue));
    myQ->head = NULL;
	myQ->tail = NULL;
    myQ->size = 0;
    return myQ;

}

int queue_enqueue(queue_t myQ, void *data)
{
    if(myQ == NULL)
    {
        printf("queue_enqueue(queue_t myQ, void *data): myQ does not exist\n");
        return -1;
    }
    if(data == NULL)
    {
        printf("queue_enqueue(queue_t myQ, void *data): data is NULL\n");
        return -1;
    }

    node* temp;
    temp = (struct node*) malloc(sizeof(node));
    temp->data = data;

    if(myQ->size == 0)
	{
		myQ->head = temp;
		myQ->tail = temp;
	}
	else
	{
		myQ->tail->next = temp;
		myQ->tail = temp;
		
	}

    myQ->size++;
    return 0;
}

int queue_dequeue(queue_t myQ, void **data)
{
    if(myQ == NULL)
    {
        printf("queue_dequeue(queue_t myQ, void *data): myQ points to NULL\n");
        return -1;
    }
    if(data == NULL)
    {
        printf("queue_dequeue(queue_t myQ, void *data): data is NULL\n");
        return -1;
    }
    if(myQ->size == 0)
    {
        printf("queue_dequeue(queue_t myQ, void *data): myQ is empty\n");
        return -1;
    }

    node* temp = myQ->head;
    *data = temp->data;

    if (myQ->size == 1)
    {
        myQ->head = NULL;
        myQ->tail = NULL;
    }
    else
    {
        myQ->head = temp->next;
    }

    free(temp);
    myQ->size--;
    return 0;
}

int queue_get(queue_t myQ, void **data)
{
	/* Check for NULL arguments */
	if (!myQ || !data)
    {
        printf("queue_get(queue_t myQ, void **data): Invalid inputs\n");
		return -1;
    }
	/* If queue is empty */
	if (myQ->size == 0)
    {
        printf("queue_get(queue_t myQ, void **data): empty queue cannot perform get\n");
		return -1;
    }
	*data = myQ->head->data;

	return 0;
}

int queue_destroy(queue_t myQ)
{
    if(myQ == NULL)
    {
        printf("queue_destroy(queue_t myQ): myQ points to NULL\n");
        return -1;
    }
    if(myQ->size != 0) 
    {
        printf("queue_destroy(queue_t myQ): myQ->size is not 0. Delete nodes first\n");
        return -1;
    }
    free(myQ);
    return 0;
}

int queue_length(queue_t myQ)
{
	if(myQ == NULL)
	{
		printf("queue_length(queue_t myQ): myQ is NULL\n");
		return -1;
	}
	return myQ->size;   
}
