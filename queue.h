#ifndef _QUEUE_H
#define _QUEUE_H

typedef struct queue* queue_t;

queue_t queue_create(void);
int queue_enqueue(queue_t myQ, void *data);
int queue_dequeue(queue_t myQ, void **data);
int queue_destroy(queue_t myQ);
int queue_get(queue_t myQ, void **data);
int queue_length(queue_t myQ);

#endif 