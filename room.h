#ifndef _ROOM_H_
#define _ROOM_H_

#include "base.h"

struct controller {
	struct BASE BASE;
	int id;
	int type;
	enum {
		OPEN = 1,
		CLOSE = 0,
		ERROR = 2,
	} state;	
};

int controller_init(struct controller *c, int id, int type);
int controller_destroy(struct controller *c);
int controller_set_status(struct controller *c, int status);
int controller_get_status(struct controller *c);
int controller_dump(struct controller *c);

struct sensor {
	struct BASE BASE;
	int id;
	int type;
	int *data;
	int n_cache;
	int current;

	int fuse;
};

int sensor_init(struct sensor *sensor, int id, int type, int n_cache);
int sensor_add_value(struct sensor *sensor, int value);
int sensor_get_fuse(struct sensor *sensor);
int sensor_destroy(struct sensor *sensor);
int sensor_dump(struct sensor *sensor);

struct room {
	struct BASE BASE;
	int id;
	struct controller *controller;
	int n_controller;
	struct sensor *sensor;
	int n_sensor;

	int n_sensor_used;
	int n_controller_used;
};

int room_init(struct room *r, int room_id, int n_controller, int n_sensor);
int room_destroy(struct room *r);
int room_add_sensor(struct room *r, struct sensor *sensor);
int room_add_controller(struct room *r, struct controller *controller);
struct sensor* room_find_sensor(struct room *r, int id);
struct controller* room_find_controller(struct room *r, int id);
int room_dump(struct room *r);

struct room_model {
	struct BASE BASE;
	struct room *room;
	int n_room;
};

int room_model_load(struct room_model *rm, char *db_file);
int room_model_destroy(struct room_model *rm);
struct room* room_model_find(struct room_model *rm, int room_id);
int room_model_dump(struct room_model *rm);
struct sensor* room_model_find_sensor(struct room_model *rm, int sensor_id);
struct controller *room_model_find_controller(struct room_model *rm, int controller_id);

#endif
