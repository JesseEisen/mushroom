#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>

#include "util.h"
#include "room.h"
#include "iniconfig.h"
#include "serial.h"

#ifdef DUMMY
void *thread_serial_sensor(void *arg)
{
	struct serial *s = (struct serial*)arg;

	struct room_model *rm = s->rm;

	while (s->is_available) {
		// rand value
		DEBUG("...\n");
		int i, j;
		for (i = 0; i < rm->n_room; i++) {
			struct room *r = &rm->room[i];
			for (j = 0; j < r->n_sensor; j++) {
				int rand_val = get_rand(0, 255);
				sensor_add_value(&r->sensor[j], rand_val);
			}
		}		

		//DEBUG("now sleep %d s\n", s->read_plc_freq);
		sleep(s->read_plc_freq);

		// TODO: read from real serial
	}

	return (void *)NULL;
}

int serial_start(struct serial *s, struct room_model *rm, struct config *config)
{
	assert(s);
	assert(rm);
	assert(config);

	memset(s, 0, sizeof(struct serial));
	DEBUG("...\n");

	// TODO:
	s->serial_fd = -1;
	sprintf(s->dev, "%s", config->serial_path);
	s->is_available = 1;
	s->read_plc_freq = config->read_plc_freq;	
	s->rm = rm;

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	int ret = pthread_create(&s->tid_serial_sensor, &attr, thread_serial_sensor, (void *)s);
	DEBUG("...\n");
	assert(ret == 0);
	pthread_attr_destroy(&attr);

	return 0;
}

int serial_stop(struct serial *s)
{
	assert(s);

	s->is_available = 0;
	s->rm = NULL;
	s->read_plc_freq = -1;
	s->serial_fd = -1;

	pthread_cancel(s->tid_serial_sensor);
	pthread_join(s->tid_serial_sensor, NULL);

	return 0;
}
#endif
