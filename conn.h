#ifndef _CONN_H_
#define _CONN_H_

void conn_init(char *endpoint);
int conn_test();

int conn_send(char *buf, int sz);
int conn_recv(char *buf, int sz);

#endif
