#include "conn.h"
#include "log.h"

#include <pthread.h>
#include <assert.h>

static void *_thread_working(void *arg)
{
	return (void *)NULL;
}

void conn_init(char *endpoint)
{
	pthread_t tid_conn;
	int ret = pthread_create(&tid_conn, NULL, _thread_working, (void *)NULL);
	assert(ret == 0);
	return;
}

int conn_test()
{
	return 0;
}

int conn_send(char *buf, int sz)
{
	return 0;	
}

int conn_recv(char *buf, int sz)
{
	return 0;
}
