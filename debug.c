#include "debug.h"
#include "pthread.h"
#include <stdarg.h>
#include <stdio.h>

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

int debug(const char *file, int line, const char *function, char *format, ...) 
{
	va_list ap;
#define MAX_PRINT_STRING 4096
	char print[MAX_PRINT_STRING];
	pthread_mutex_lock(&mut);
	time_t ts; 
	time(&ts); 
	struct tm *tm = localtime(&ts); 
	char tstamp[255]; 
	sprintf(tstamp, "{%02d-%02d-%02d %02d:%02d:%02d}", 
			tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec); 
	fprintf(stderr, "%s:[DEBUG]:%s:%d:%s():", tstamp, file, line, function); 

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	fflush(stdout);
	pthread_mutex_unlock(&mut);

	return 0;
}

void no_op() {}
