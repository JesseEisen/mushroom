#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "util.h"

struct room_model;
struct config;

struct serial {
	int serial_fd;
	char dev[255];
	int is_available;
	struct recv recv;
	int read_plc_freq;
	struct room_model *rm;
	pthread_t tid_serial_sensor;
};

int serial_start(struct serial *s, struct room_model *rm, struct config *config);
int serial_stop(struct serial *s);

#endif
