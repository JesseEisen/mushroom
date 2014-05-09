#ifndef _UTIL_H_
#define _UTIL_H_

int nwrite(int fd, char *buf, int sz);
int nread (int fd, char *buf, int sz);
int do_connect(char *ip, int port);
int do_listen(const char * host, int port, int backlog);
int parse_endpoint(char *endpoint, char *ip, int *port);

#endif
