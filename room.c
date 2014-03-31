#include "base.h"
#include "room.h"
#include "sqlite3.h"
#include "log.h"

int controller_init(struct controller *c, int id, int type)
{
	assert(c);
	assert(id >= 0);

	c->id = id;
	c->type = type;
	c->state = CLOSE;

	c->BASE.magic = MAGIC_NUMBER;

	return 0;
}

int controller_destroy(struct controller *c)
{
	assert(c);
	if (c->BASE.magic != MAGIC_NUMBER) {
		log_err("controller do not init.\n");
		return -1;
	}
	c->id = -1;
	c->state = CLOSE;

	return 0;
}


int controller_set_status(struct controller *c, int status)
{
	assert(c);
	if (c->BASE.magic != MAGIC_NUMBER) {
		log_err("controller do not init.\n");
		return -1;
	}
	if (!(status == OPEN || status == CLOSE || status == ERROR)) {
		log_err("status error\n");
		return -1;
	}

	c->state = status;

	return 0;
}

int controller_get_status(struct controller *c)
{
	assert(c);
	if (c->BASE.magic != MAGIC_NUMBER) {
		log_err("controller do not init.\n");
		return -1;
	}

	return c->state;
}

int controller_dump(struct controller *c)
{
	assert(c);
	if (c->BASE.magic != MAGIC_NUMBER) {
		log_err("sensor do not init\n");
		return -1;
	}

	DEBUG("Controller: id: %d, type:%d, status:%d\n",
			c->id, c->type, c->state); 	

	return 0;
}

int sensor_init(struct sensor *sensor, int id, int type, int n_cache)
{
	assert(sensor);
	assert(id >= 0);
	assert(n_cache > 0);	

	sensor->id = id;
	sensor->type = type;
	sensor->data = malloc(sizeof(int)*n_cache);
	assert(sensor->data);
	memset(sensor->data, 0, sizeof(int)*n_cache);

	sensor->n_cache = n_cache;
	sensor->current = 0;
	sensor->fuse = 1 << 16;

	sensor->BASE.magic = MAGIC_NUMBER;

	return 0;
}

int sensor_add_value(struct sensor *sensor, int value)
{
	assert(sensor);
	if (sensor->BASE.magic != MAGIC_NUMBER) {
		log_err("sensor do not init\n");
		return -1;
	}

	if (sensor->current >= sensor->n_cache) {
		sensor->current = 0;
	}

	sensor->data[sensor->current++] = value;

	int i;
	int sum = 0;
	for (i = 0; i < sensor->current; i++) {
		sum += sensor->data[i];
	}

	int avg = sum / sensor->current;
	if (sensor->fuse != (1 << 16)) {
		avg = (sensor->fuse + avg)/2;
	}

	sensor->fuse = avg;

	return 0;
}

int sensor_get_fuse(struct sensor *sensor)
{
	assert(sensor);
	if (sensor->BASE.magic != MAGIC_NUMBER) {
		log_err("sensor do not init\n");
		return -1;
	}
	return sensor->fuse;
}

int sensor_destroy(struct sensor *sensor)
{
	assert(sensor);
	if (sensor->BASE.magic != MAGIC_NUMBER) {
		log_err("sensor do not init\n");
		return -1;
	}

	sensor->id = -1;
	free(sensor->data);
	sensor->data = NULL;
	sensor->n_cache = -1;
	sensor->current = -1;
	sensor->fuse = 1 << 16;

	return 0;
}

int sensor_dump(struct sensor *sensor)
{
	assert(sensor);
	if (sensor->BASE.magic != MAGIC_NUMBER) {
		log_err("sensor do not init\n");
		return -1;
	}

	DEBUG("Sensor: id: %d, type:%d, n_cache:%d, current:%d, fuse:%d\n",
			sensor->id, sensor->type, sensor->n_cache, sensor->current, sensor->fuse); 	

	int i;
	DEBUG("data:|");	
	for (i = 0; i < sensor->current; i++) {
		printf("%d|", sensor->data[i]);
	}

	printf("\n");

	return 0;
}

int room_init(struct room *r, int room_id, int n_controller, int n_sensor)
{
	assert(r);

	r->id = room_id;
	r->controller = malloc(sizeof(struct controller)*n_controller);
	r->sensor = malloc(sizeof(struct sensor)*n_sensor);
	assert(r->controller);
	assert(r->sensor);

	r->n_controller = n_controller;
	r->n_sensor = n_sensor;

	r->n_controller_used = 0;
	r->n_sensor_used = 0;

	r->BASE.magic = MAGIC_NUMBER;

	return 0;
}

int room_destroy(struct room *r)
{
	//TODO: pay attention to this free point!

	assert(r);
	if (r->BASE.magic != MAGIC_NUMBER) {
		log_err("room do not init.\n");
		return -1;
	}

	// TODO: use sensor_destroy to free.
	int i;
	if (r->sensor) {
		for (i = 0; i < r->n_sensor_used; i++) {
			sensor_destroy(&r->sensor[i]);
		}

		free(r->sensor);
	}

	if (r->controller) {
		for (i = 0; i < r->n_controller_used; i++) {
			controller_destroy(&r->controller[i]);
		}

		free(r->controller);
	}

	r->n_controller = 0;
	r->n_sensor = 0;
	r->sensor = NULL;
	r->controller = NULL;

	r->n_sensor_used = 0;
	r->n_controller_used = 0;

	return 0;
}

int room_add_sensor(struct room *r, struct sensor *sensor)
{
	assert(r);
	assert(sensor);
	if (r->BASE.magic != MAGIC_NUMBER) {
		log_err("room do not init.\n");
		return -1;
	}

	if (r->n_sensor_used >= r->n_sensor) {
		log_err("too many sensor\n");
		return -1;
	}

	struct sensor *s = &r->sensor[r->n_sensor_used++];
	sensor_init(s, sensor->id, sensor->type, sensor->n_cache);

	return 0;
}

int room_add_controller(struct room *r, struct controller *controller)
{
	assert(r);
	assert(controller);
	if (r->BASE.magic != MAGIC_NUMBER) {
		log_err("room do not init.\n");
		return -1;
	}

	if (r->n_controller_used >= r->n_controller) {
		log_err("too many controller\n");
		return -1;
	}

	struct controller *c = &r->controller[r->n_controller_used++];
	controller_init(c, controller->id, controller->type);

	return 0;
}


int room_dump(struct room *r)
{
	assert(r);
	if (r->BASE.magic != MAGIC_NUMBER) {
		log_err("room do not init.\n");
		return -1;
	}

	DEBUG("*****************Room[%d]*********************\n", r->id);
	int i;
	for (i = 0; i < r->n_sensor_used; i++) {
		sensor_dump(&r->sensor[i]);
	}

	for (i = 0; i < r->n_controller_used; i++) {
		controller_dump(&r->controller[i]);
	}
	return 0;	
}

struct sensor* room_find_sensor(struct room *r, int id)
{
	assert(r);
	if (r->BASE.magic != MAGIC_NUMBER) {
		log_err("room do not init.\n");
		return NULL;
	}
	int i;
	for (i = 0; i < r->n_sensor_used; i++) {
		if (r->sensor[i].id == id) return &r->sensor[i];
	}

	return NULL;
}

struct controller* room_find_controller(struct room *r, int id)
{
	assert(r);
	if (r->BASE.magic != MAGIC_NUMBER) {
		log_err("room do not init.\n");
		return NULL;
	}
	int i;
	for (i = 0; i < r->n_controller_used; i++) {
		if (r->controller[i].id == id) return &r->controller[i];
	}

	return NULL;
}

int room_model_load(struct room_model *rm, char *db_file)
{
	assert(rm);
	assert(db_file);
	rm->BASE.magic = MAGIC_NUMBER;

	sqlite3 *conn;
	int error;
	error = sqlite3_open(db_file, &conn);
	assert(error == 0);

	sqlite3_stmt    *res;
	int     rec_count = 0;
	const char      *tail;

	// 1. get room info
	char *sql = "select tb_sensor.room_id, tb_sensor.sensor_num, tb_controller.controller_num "\
		     "from (select room_id, count(sensor_id) as sensor_num "\
		     "from sensor group by room_id) as tb_sensor "\
		     "left join (select room_id, count(controller_id) as controller_num "\
		     "	       from controller group by room_id)    as tb_controller "\
		     "on tb_sensor.room_id = tb_controller.room_id;";

	int len = strlen(sql);

	error = sqlite3_prepare_v2(conn, sql, len, &res, NULL);
	assert(error == SQLITE_OK);
	rec_count = 0;
	while (sqlite3_step(res) == SQLITE_ROW) {
		rec_count++;
	}

	sqlite3_reset(res);

	rm->n_room = rec_count;
	rm->room = malloc(sizeof(struct room)*(rm->n_room));
	assert(rm->room);

	int i = 0;
	while (sqlite3_step(res) == SQLITE_ROW) {
		int n_sensor = sqlite3_column_int(res, 1);
		int n_controller = sqlite3_column_int(res, 2);
		int id = sqlite3_column_int(res, 0);
		room_init(&rm->room[i], id, n_controller, n_sensor);
		i++;
	}

	// TODO: check this function call
	error = sqlite3_finalize(res);
	if (error != SQLITE_OK) {
		log_err("error: %s\n", sqlite3_errmsg(conn));
		exit(-1);
	}	

	//	// 2. init every sensor and controller
	sql = "select sensor_id, room_id, sensor_type from sensor;";
	len = strlen(sql);
	//	
	error = sqlite3_prepare_v2(conn, sql, len, &res, NULL);
	assert(error == SQLITE_OK);
	//
	while (sqlite3_step(res) == SQLITE_ROW) {
		int room_id = sqlite3_column_int(res, 1);
		struct room *r = room_model_find(rm, room_id);
		assert(r);
		//		
		int id = sqlite3_column_int(res, 0);
		int type = sqlite3_column_int(res, 2);

#define SENSOR_CACHE 10
		struct sensor sensor;
		sensor_init(&sensor, id, type, SENSOR_CACHE);
		room_add_sensor(r, &sensor);
		sensor_destroy(&sensor);
	}

	error = sqlite3_finalize(res);
	if (error != SQLITE_OK) {
		log_err("error: %s\n", sqlite3_errmsg(conn));
		exit(-1);
	}	

	sql = "select controller_id, room_id, controller_type from controller";
	len = strlen(sql);
	//	
	error = sqlite3_prepare_v2(conn, sql, len, &res, NULL);
	assert(error == SQLITE_OK);
	//		
	while (sqlite3_step(res) == SQLITE_ROW) {
		int room_id = sqlite3_column_int(res, 1);
		struct room *r = room_model_find(rm, room_id);
		assert(r);

		int id = sqlite3_column_int(res, 0);
		int type = sqlite3_column_int(res, 2);
		struct controller controller;

		controller_init(&controller, id, type);
		room_add_controller(r, &controller);
		controller_destroy(&controller);
	}

	error = sqlite3_finalize(res);
	if (error != SQLITE_OK) {
		log_err("error: %s\n", sqlite3_errmsg(conn));
		exit(-1);
	}

	sqlite3_close(conn);
	return 0;
}

int room_model_destroy(struct room_model *rm)
{
	assert(rm);
	if (rm->BASE.magic != MAGIC_NUMBER) {
		log_err("room_model do not init\n");
		return -1;
	}

	int i;
	for (i = 0; i < rm->n_room; i++) {
		room_destroy(&rm->room[i]);
	}

	free(rm->room);

	return 0;
}

struct room* room_model_find(struct room_model *rm, int room_id)
{
	assert(rm);
	assert(room_id >= 0);

	if (rm->BASE.magic != MAGIC_NUMBER) {
		log_err("room_model do not init\n");
		return NULL;
	}

	int i;
	for (i = 0; i < rm->n_room; i++) {
		if (rm->room[i].id == room_id) return &rm->room[i];
	}

	return NULL;
}

int room_model_dump(struct room_model *rm)
{
	assert(rm);	
	if (rm->BASE.magic != MAGIC_NUMBER) {
		log_err("room_model do not init\n");
		return -1;
	}

	int i;
	for (i = 0; i < rm->n_room; i++) {
		room_dump(&rm->room[i]);
	}

	return 0;
}


struct sensor* room_model_find_sensor(struct room_model *rm, int sensor_id)
{
	assert(rm);
	if (rm->BASE.magic != MAGIC_NUMBER) {
		log_err("room_model do not init\n");
		return NULL;
	}
	int i;
	struct sensor *result;
	for (i = 0; i < rm->n_room; i++) {
		result = room_find_sensor(&rm->room[i], sensor_id);
		if (result) return result;
	}

	return NULL;
}

struct controller *room_model_find_controller(struct room_model *rm, int controller_id)
{
	assert(rm);
	if (rm->BASE.magic != MAGIC_NUMBER) {
		log_err("room_model do not init\n");
		return NULL;
	}
	int i;
	struct controller *result;
	for (i = 0; i < rm->n_room; i++) {
		result = room_find_controller(&rm->room[i], controller_id);
		if (result) return result;
	}

	return NULL;
}
