#include "log.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include "debug.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static FILE *warning_fp;
static FILE *info_fp;
static FILE *trace_fp;
static FILE *err_fp;

void log_print(int level, int use, const char *file, int line, const char *function, const char *format, ...)
{
#ifdef LOG_PRINT

	va_list ap;

#define MAX_SIZE_PRINT 4096
	char print[MAX_SIZE_PRINT];
	memset(print, 0, MAX_SIZE_PRINT);

	if (level < 0 || !use)
		return;

	char info []   = "[INFO]";
	char trace[]   = "[TRACE]";
	char warning[] = "[WARNING]";
	char error[]   = "[ERROR]";

	char *t;
	FILE *log_fp;
	switch (level) {
		case 0:
			if (info_fp) {
				log_fp = info_fp;
			}
			t = info;
			break;
		case 1:
			if (trace_fp) {
				log_fp = trace_fp;
			}
			t = trace;
			break;
		case 2:
			if (warning_fp) {
				log_fp = warning_fp;
			}
			t = warning;
			break;
		case 3:
			if (err_fp) {
				log_fp = err_fp;
			}
			t = error;
			break;	
		default:
			log_fp = NULL;
			return;
	}

	pthread_mutex_lock(&lock);
	sprintf(print, "%s:%s:%d:", t, file, line);

	t = print + strlen(print);

	va_start(ap, format);
	vsprintf(t, format, ap);
	va_end(ap);

	time_t ts;
	time(&ts);
	struct tm *tm = localtime(&ts);

	char tstamp[255];
	sprintf(tstamp, "{%02d-%02d-%02d %02d:%02d:%02d}",
			tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);	

	if (log_fp) {
		fprintf(log_fp, "%s:%s", tstamp, print);
		fflush(log_fp);
	}

	pthread_mutex_unlock(&lock);
#endif
}

void log_init(char *info, char *warning, char *trace, char *err)
{
#ifdef LOG_PRINT
	// TODO: set thread deattach
	if (info) {
		info_fp = fopen(info, "a+");
		assert(info_fp);
	}

	if (trace) {
		trace_fp = fopen(trace, "a+");
		assert(trace_fp);
	}

	if (warning) {
		warning_fp = fopen(warning, "a+");
		assert(warning_fp);
	}

	if (err) {
		err_fp = fopen(err, "a+");
		assert(err_fp);
	}
#endif
}

void log_destroy()
{
	if (info_fp) {
		fclose(info_fp);
	}

	if (trace_fp) {
		fclose(trace_fp);
	}

	if (warning_fp) {
		fclose(warning_fp);
	}

	if (err_fp) {
		fclose(err_fp);
	}
}
