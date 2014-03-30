#ifndef _BASE_H_
#define _BASE_H_

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>

#include "debug.h"

#define MAGIC_NUMBER 0xFA
struct BASE {
	int magic;
};

#define malloc(x) calloc(1, x)

#endif
