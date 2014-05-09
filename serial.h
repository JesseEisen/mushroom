#ifndef _SERIAL_H_
#define _SERIAL_H_

#define SERIAL_MAX_BUF 4096

void serial_init(char *dev);
int serial_test();

struct serial_request {
	void *ud;
	int (*cb)(void *ud, char *msg, int sz);
	char buf[SERIAL_MAX_BUF];
	int sz;
};

int serial_send(struct serial_request *req);

// helper function
int getRCommandBuffer(char *buf, int *sz, int address, int num);
int getWCommandBuffer(char *buf, int *sz, int address, int num, char *data);

#endif
