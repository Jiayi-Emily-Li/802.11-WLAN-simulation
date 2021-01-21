#include <stdlib.h>
#include <stdio.h>

#include "gel.h"

typedef struct node 
{
    event_t event;
    struct node* prev;
    struct node* next;
}node;

struct dll 
{
    int size;
    node* head;
    node* tail;
};

dll_t gel_create()
{
    dll_t gel;
    gel = (struct dll*) malloc(sizeof(struct dll));
    gel->head = NULL;
	gel->tail = NULL;
	gel->size = 0;
    return gel;
}

int gel_insert(dll_t gel, event_t event)
{
    if (gel == NULL)
    {
        printf("int gel_insert(dll_t gel, event_t event): gel points to NULL");
        return -1;
    }

    node* temp;
    temp = (struct node*) malloc(sizeof(struct node));
    temp->event = event;

    if (gel->size == 0) 
    {
		temp->next = NULL;
		temp->prev = NULL;
		gel->head = temp;
		gel->tail =temp;
		gel->size = 1;
		return 0;
	} 
    else
    {
        node* ptr =gel->head;
        while(ptr)
        {
            if(temp->event->timeS == ptr->event->timeS)
            {
                //printf("COLLISION!!!: gel_insert(dll_t gel, event_t event): Two events at the same time\n");
                printf("%d, %d, %lf\n", ptr->event->srcHost, ptr->event->type, ptr->event->timeS);
                return -2;
            }
            if(temp->event->timeS > ptr->event->timeS)
            {
                /*
                 * If ptr points to the tail
                 * Add the node as the tail
                 */
                if(!ptr->next) 
                {
                    ptr->next = temp;
					temp->prev = ptr;					
                    temp->next = NULL;
					gel->tail = temp;
					gel->size++;
					return 0;
                }
                else
                    ptr = ptr->next;   
            }
            else
            {
                /*
                 * If ptr points to the head
                 * Add the node as the head
                 */
                if (!ptr->prev) 
                {
                    ptr->prev = temp;
                    temp->next = ptr;
                    temp->prev = NULL;
                    gel->head = temp;
                    gel->size++;
                    return 0;
                }
                else{
					temp->prev = ptr->prev;
                    ptr->prev->next = temp;
					temp->next = ptr;
					ptr->prev = temp;
					gel->size++;	
                    return 0;
                }
            }
        }
    }

    printf("int gel_insert(dll_t gel, event_t event): Inaccessible\n");
    return -1;
}

int gel_remove(dll_t gel, event_t* event)
{
    if(gel == NULL)
    {
        printf("gel_remove(dll_t gel, event_t event): gel points to NULL\n");
        return -1;
    }
    if(event == NULL)
    {
        printf("gel_remove(dll_t gel, event_t event): data is NULL\n");
        return -1;
    }
    if(gel->size == 0)
    {
        printf("gel_remove(dll_t gel, event_t event): gel is empty\n");
        return -1;
    }
    
    node* temp = gel->head;
    *event = temp->event;

    if (gel->size == 1)
    {
        gel->head = NULL;
        gel->tail = NULL;
    }
    else
    {
        gel->head = temp->next;
        gel->head->prev = NULL;
    }

    free(temp);
    gel->size--;
    return 0;
}

int gel_destroy(dll_t gel) 
{
    if(gel == NULL)
    {
        printf("gel_destroy(dll_t gel): gel points to NULL\n");
        return -1;
    }
    if (gel->size != 0)
    {
		printf("gel_destroy(dll_t gel): gel->size is not 0. Delete nodes first\n");
        return -1;
    }

	free(gel);
	return 0;
}

int gel_length(dll_t gel)
{
	if(gel == NULL)
	{
		printf("gel_length(dll_t gel): gel is NULL\n");
		return -1;
	}
	return gel->size;   
}

event_t create_event(int srcHost, int destHost, double timeS, double timeL, int type , double backoff, double delay, int attempts)
{
    event_t temp = malloc(sizeof(struct event));
    temp->srcHost = srcHost;
    temp->destHost = destHost;
    temp->timeS = timeS;
    temp->timeL = timeL;
    temp->type = type;
    temp->backoff = backoff;
    temp->delay = delay;
    temp->attempts = attempts;

    return temp;
}