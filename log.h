#ifndef _LOG_H_
#define _LOG_H_

void log_init(char *logfile, int debug_info, int debug_warning, int debug_err,
				int log_info, int log_warning, int log_err);
int log_test();

#include <stdio.h>

void log_print(int level,
                const char *file,
                int line,
                const char *format,
                ...);

#define log_info(...) \
        do { log_print(0, __FILE__, __LINE__, __VA_ARGS__); } while(0)

#define log_warning(...) \
        do { log_print(1, __FILE__, __LINE__, __VA_ARGS__); } while(0)

#define log_err(...) \
        do { log_print(2, __FILE__, __LINE__, __VA_ARGS__); } while(0)

#endif
