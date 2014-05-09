#include "queue.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#define true 1
#define false 0

struct node {
	void* key;
	int priority;
	struct node* next;
};

struct queue_entry {
	int priority;
	struct node* n;
};

struct queue {
	struct queue_entry entry[PRI_MAX];
	struct node* last[PRI_MAX];
	struct node* buf_pool;
	int ent_count;
	pthread_mutex_t lock;
	pthread_cond_t cv;
	int is_available;
	int size;
};

static void put_buf(struct queue *p, void* buf);
static void create_pool(struct queue **p, uint32_t num);
static void* get_buf(struct queue *p);
static void add_a_node(struct queue *p, struct node** last, struct node** m, void* key, int priority);

static void add_a_node(struct queue* p, struct node** last, struct node** m, void* key, int prio)
{
	assert(p);

	pthread_mutex_lock(&p->lock);
	struct node *n = NULL;

	n = (struct node*)get_buf(p);

	if (NULL == n) {
		pthread_cond_wait(&p->cv, &p->lock);
		n = (struct node*)get_buf(p);
	}
	n->key = key;
	n->priority = prio;
	n->next = NULL;

	if (NULL == *m) {
		*m = n;
	} else {
		(*last)->next = n;
	}

	*last = n;

	p->is_available = true;
	pthread_cond_signal(&p->cv);
	p->size++;
	pthread_mutex_unlock(&p->lock);
}

static void* get_buf(struct queue *p)
{
	struct node* head = p->buf_pool;

	if(p->buf_pool != NULL) {
		p->buf_pool = head->next;
		return head;
	} else {
		return NULL;
	}
}

static void put_buf(struct queue* p, void* buf)
{
	if (p->buf_pool) {
		struct node* head = (struct node*)buf;
		head->next = p->buf_pool;
		p->buf_pool = head;
	} else {
		p->buf_pool = buf;
	}
}

static void create_pool(struct queue** p, uint32_t num)
{
	struct node* head= NULL;
	struct node* temp = NULL;

	int i = 0;

	head = malloc(sizeof(struct node)); 
	memset(head, 0, sizeof(struct node));
	assert(head);

	temp = head;

	for(i = 1; i < num; i++) {
		temp->next = malloc(sizeof(struct node));
		memset(temp->next, 0, sizeof(struct node));
		assert(temp->next);
		temp = temp->next;
	}
	temp->next = NULL;

	if (NULL == (*p)->buf_pool) {
		(*p)->buf_pool = head;
	}
}

struct queue* queue_create()
{
	struct queue *p = malloc(sizeof(struct queue));
	assert(p);

	int i = 0;

	for(i = 0; i < PRI_MAX; i++) {
		p->entry[i].priority = i;
		p->entry[i].n = NULL;
		p->last[i] = NULL;
	}

	create_pool(&p, BUF_POOL_SIZE);

	memset ( &(p->lock), 0, sizeof(pthread_mutex_t));
	memset ( &(p->cv), 0, sizeof(pthread_cond_t));
	pthread_mutex_init(&(p->lock), NULL);
	pthread_cond_init(&(p->cv), NULL);

	p->is_available = false;
	p->ent_count = PRI_MAX;
	p->size = 0;

	return p;
}

void queue_push(struct queue *p, void* key, int prio)
{
	assert(p);
	assert(prio < PRI_MAX);

	add_a_node(p, &(p->last[prio]), &(p->entry[prio].n),
			key, prio);
}

void *queue_pop(struct queue *p, int* pri)
{
	assert(p);

	pthread_mutex_lock(&p->lock);
	int i = 0;
	struct node* temp = NULL;
	void *key;

wait_again:
	while (false == p->is_available) {
		pthread_cond_wait(&p->cv, &p->lock);
	}

	for (i = 0; i < PRI_MAX; i++) {
		if (NULL != p->entry[i].n) {
			temp = (p->entry[i].n);

			key = p->entry[i].n->key;
			if (pri) *pri = p->entry[i].n->priority;

			p->entry[i].n = temp->next;
			put_buf(p, temp);
			pthread_cond_signal(&p->cv);
			p->size--;
			pthread_mutex_unlock(&p->lock);
			return key;
		}
	}
	p->is_available = false;
	goto wait_again;
}

void queue_release(struct queue *p)
{
	struct node *n = p->buf_pool;

	while(n) {
		struct node* temp = n;
		n = n->next;
		free(temp);
	}

	pthread_mutex_destroy(&(p->lock));
	pthread_cond_destroy(&(p->cv));
	free(p);
}

int queue_size(struct queue *q)
{
	// something problems
	// TODO
	return q->size;
}
