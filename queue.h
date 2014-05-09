#ifndef _QUEUE_H_
#define _QUEUE_H_

#define PRI_MAX 10
#define BUF_POOL_SIZE 4096

struct queue;

struct queue* queue_create();
void   queue_release(struct queue*);
void   queue_push(struct queue*, void* key, int priority);
void*  queue_pop(struct queue*, int* priority);
int    queue_size(struct queue*);

#endif
