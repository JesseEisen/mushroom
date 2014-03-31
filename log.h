#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>

#define LOG_PRINT

void log_init(char *info, char *warning, char *trace, char *err);

void log_print(int level, 
		int use, 
		const char *file, 
		int line, 
		const char *function, 
		const char *format, 
		...);

#define log_info(...) \
	do { log_print(0, 1, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); } while(0)

#define log_trace(...) \
	do { log_print(1, 1, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); } while(0)

#define log_warning(...) \
	do { log_print(2, 1, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); } while(0)

#define log_err(...) \
	do { log_print(3, 1, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); } while(0)

#endif
