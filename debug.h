#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>

#define DEBUG_PRINT
int debug(const char *file, int line, const char *function, char *format, ...);

#if defined(DEBUG_PRINT)
#define DEBUG(...) \
	do { debug(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); } while(0)
#else
void no_op();
#define DEBUG(...) no_op()
#endif /* NO_DEBUG */

#endif
