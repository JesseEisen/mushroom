#include "log.h"
#include "queue.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

static FILE *_L = NULL;
static struct queue *_Q = NULL;

static int _debug_info = 0;
static int _debug_warning = 0;
static int _debug_err = 0;

static int _log_info = 0;
static int _log_warning = 0;
static int _log_err = 0;

struct _log_message {
	char print[255];
};

static int MAGIC_NO = 0;

static void *_thread_working(void *arg)
{
	while (1) {
		struct _log_message *msg = queue_pop(_Q, NULL);
		fprintf(_L, "%s\n", msg->print);
		free(msg);
	}

	return (void *)NULL;	
}

void log_init(char *logfile, int debug_info, int debug_warning, int debug_err,
				int log_info, int log_warning, int log_err)
{	
	if (_L || _Q) {
		return;
	}
	_Q = queue_create();
	_L = fopen(logfile, "a+");
	if (_L == NULL) {
		fprintf(stderr, "[log] %s\n", strerror(errno));
		return;
	}

	_debug_info = debug_info;
	_debug_warning = debug_warning;
	_debug_err = debug_err;
	
	_log_info = log_info;
	_log_warning = log_warning;
	_log_err = log_err;

	pthread_t tid_log;
	int ret = pthread_create(&tid_log, NULL, _thread_working, (void *)NULL);
	assert(ret == 0);

	MAGIC_NO = 0x1234;
}

int log_test()
{
	if (MAGIC_NO != 0x1234) {
		return -1;
	}
	return 0;
}

void log_print(int level,
                const char *file,
                int line,
                const char *format,
                ...)
{
//	char tmp[255];
//	switch (level) {				
	// TODO
	fprintf(stdout, "[test log]:%s:%d:Hello World\n", file, line);
	fprintf(_L,     "[test log]:%s:%d:Hello World\n", file, line);
	fflush(stdout);
	fflush(_L);
					
}
