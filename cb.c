#include "base.h"
#include "cb.h"
#include "socket_client.h"
#include "proto/service.pb-c.h"
#include "room.h"
#include "log.h"

#define CALLBACK(NAME) \
	int NAME(struct socket_client *sc, void *ud, MessageHeader *header, void *msg, int sz)

#define CALLBACK_ENTRY(NAME) \
	{MESSAGE_ID(NAME), NULL, NAME}

#define MESSAGE_ID(NAME) \
	MESSAGE__ID__##NAME


CALLBACK(READ_TIME_RESPONSE);
CALLBACK(READ_CONF_RESPONSE);
CALLBACK(UPDATE_TIME);
CALLBACK(UPDATE_CONF);
CALLBACK(REBOOT);
CALLBACK(READ_CONTROLLER_STATE);
CALLBACK(UPDATE_CONTROLLER_STATE);
CALLBACK(READ_SENSOR_DATA);
CALLBACK(PING);

#define MAX_ENTRY 1024

struct callback *cb_entry;
int n_cb_entry;

struct protobuf_message *pb_entry;
int n_pb_entry;

pthread_mutex_t lock_cb_entry = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_pb_entry = PTHREAD_MUTEX_INITIALIZER;

int message_header_copy(MessageHeader *h1_src, MessageHeader *h2_dst)
{
	assert(h1_src);
	assert(h2_dst);
	
	h2_dst->message_id = h1_src->message_id;

	DEBUG("h1_src: %d, %d, %d, %d\n", h1_src->has_session, h1_src->has_version, h1_src->has_connection, h1_src->has_source);
	DEBUG("h1_src: session:%d, version:%d, connection:%d, source:%d\n",
		h1_src->session, h1_src->version, h1_src->connection, h1_src->source);
	
	if (h1_src->has_session) {
		h2_dst->has_session = 1;
		h2_dst->session = h1_src->session;
	}
	
	if (h1_src->has_type) {
		h2_dst->has_type = 1;
		h2_dst->type = h1_src->type;
	}
	
	if (h1_src->has_version) {
		h2_dst->has_version = 1;
		h2_dst->version = h1_src->version;
	}
	
	if (h1_src->has_connection) {
		h2_dst->has_connection = 1;
		h2_dst->connection = h1_src->connection;
	}
	
	if (h1_src->has_source) {
		h2_dst->has_source = 1;
		h2_dst->source = h1_src->source;
	}

	return 0;
}


int protobuf_message_init(void)
{
	pb_entry = malloc(sizeof(struct protobuf_message)*MAX_ENTRY);
	assert(pb_entry);
	n_pb_entry = 0;

	PROTOBUF_MESSAGE_REGISTER("Room", room_init, 
						room__get_packed_size,
						room__pack,
						room__unpack,
						room__free_unpacked);
	PROTOBUF_MESSAGE_REGISTER("Sensor", sensor__init,
						sensor__get_packed_size,
						sensor__pack,
						sensor__unpack,
						sensor__free_unpacked);

	PROTOBUF_MESSAGE_REGISTER("SynTime", syn_time__init,
						syn_time__get_packed_size,
						syn_time__pack,
						syn_time__unpack,
						syn_time__free_unpacked);

	PROTOBUF_MESSAGE_REGISTER("ResponseCode", response_code__init,
							response_code__get_packed_size,
							response_code__pack,
							response_code__unpack,
							response_code__free_unpacked);
	PROTOBUF_MESSAGE_REGISTER("Config", config__init,
						config__get_packed_size,
						config__pack,
						config__unpack,
						config__free_unpacked);

	PROTOBUF_MESSAGE_REGISTER("ConfigList", config_list__init,
							config_list__get_packed_size,
							config_list__pack,
							config_list__unpack,
							config_list__free_unpacked);

	PROTOBUF_MESSAGE_REGISTER("Controller", controller__init,
							controller__get_packed_size,
							controller__pack,
							controller__unpack,
							controller__free_unpacked);

	PROTOBUF_MESSAGE_REGISTER("SensorData", sensor_data__init,
							sensor_data__get_packed_size,
							sensor_data__pack,
							sensor_data__unpack,
							sensor_data__free_unpacked);

	PROTOBUF_MESSAGE_REGISTER("Ping", ping__init,
						ping__get_packed_size,
						ping__pack,
						ping__unpack,
						ping__free_unpacked);

	PROTOBUF_MESSAGE_REGISTER("RoomConf", room_conf__init,
						room_conf__get_packed_size,
						room_conf__pack,
						room_conf__unpack,
						room_conf__free_unpacked);

	PROTOBUF_MESSAGE_REGISTER("Init", init__init,
						init__get_packed_size,
						init__pack,
						init__unpack,
						init__free_unpacked);
	
	return 0;
}

int protobuf_message_register(char *name, pb_init init,
						pb_get_packed_size get_packed_size,
						pb_pack pack,
						pb_unpack unpack,
						pb_free_unpacked free_unpacked)
{
	if (name == NULL) {
		log_err("name error\n");
		return -1;
	}

	pthread_mutex_lock(&lock_pb_entry);

	struct protobuf_message *pbe = &pb_entry[n_pb_entry++];

	if (n_pb_entry > MAX_ENTRY) {
		log_err("too many callback!\n");
		exit(1);
	}

	sprintf(pbe->name, "%s", name);

	pbe->init = init;
	pbe->get_packed_size = get_packed_size;
	pbe->pack = pack;
	pbe->unpack = unpack;
	pbe->free_unpacked = free_unpacked;

	pthread_mutex_unlock(&lock_pb_entry);

	return 0;
}

struct protobuf_message* protobuf_message_find(char *name)
{
	if (name == NULL) {
		log_err("name error\n");
		return NULL;
	}
	
	pthread_mutex_lock(&lock_pb_entry);
	
	int i;
	for (i = 0; i < n_pb_entry; i++) {
		struct protobuf_message *pbe = &pb_entry[i];
		if (strcmp(pbe->name, name) == 0) {
			pthread_mutex_unlock(&lock_pb_entry);
			return pbe;
		}
	}

	pthread_mutex_unlock(&lock_pb_entry);
		
	return NULL;
}

void protobuf_message_dump(void)
{
	int i;
	
	for (i = 0; i < n_pb_entry; i++) {
		struct protobuf_message *pbe = protobuf_message_find(pb_entry[i].name);	
		DEBUG("pbe: name:%s, init:%p, get_packed_size:%p, pack:%p, unpack:%p, free_unpacked:%p\n",
			pbe->name, pbe->init, pbe->get_packed_size, pbe->pack, pbe->unpack, pbe->free_unpacked);
	}

}

void protobuf_message_destroy(void)
{
	assert(pb_entry);
	free(pb_entry);
	pb_entry = NULL;
	n_pb_entry = 0;
}

int callback_register(int message_id, void *ud, int ud_option, MessageCallback cb)
{
	assert(message_id >= 0);
	assert(cb);
	assert(cb_entry);

	pthread_mutex_lock(&lock_cb_entry);

	struct callback *cbe = &cb_entry[n_cb_entry++];
	if (n_cb_entry > MAX_ENTRY) {
		log_err("too many callback!\n");
		exit(1);
	}
	
	if (ud == NULL) {
		cbe->ud_option = -1;
	} else {
		cbe->ud_option = ud_option;
	}

	cbe->message_id = message_id;
	cbe->ud = ud;
	cbe->cb = cb;

	pthread_mutex_unlock(&lock_cb_entry);

	return 0;
}

void callback_init(void)
{
	cb_entry = malloc(sizeof(struct callback)*MAX_ENTRY);
	assert(cb_entry);
	n_cb_entry = 0;
	callback_register(MESSAGE_ID(READ_TIME_RESPONSE),      NULL, -1, READ_TIME_RESPONSE);
	callback_register(MESSAGE_ID(READ_CONF_RESPONSE),      NULL, -1, READ_CONF_RESPONSE);
	callback_register(MESSAGE_ID(UPDATE_TIME),             NULL, -1, UPDATE_TIME);
	callback_register(MESSAGE_ID(UPDATE_CONF),             NULL, -1, UPDATE_CONF);
	callback_register(MESSAGE_ID(REBOOT),                  NULL, -1, REBOOT);
	callback_register(MESSAGE_ID(READ_CONTROLLER_STATE),   NULL, -1, READ_CONTROLLER_STATE);
	callback_register(MESSAGE_ID(UPDATE_CONTROLLER_STATE), NULL, -1, UPDATE_CONTROLLER_STATE);
	callback_register(MESSAGE_ID(READ_SENSOR_DATA),        NULL, -1, READ_SENSOR_DATA);
	callback_register(MESSAGE_ID(PING),                    NULL, -1, PING);

	printf("n_cb_entry:%d\n", n_cb_entry);
}

struct callback *callback_find(int message_id)
{

	assert(cb_entry);
	assert(message_id >= 0);

	pthread_mutex_lock(&lock_cb_entry);

	int i;
	
	for (i = 0; i < n_cb_entry; i++) {
		if (cb_entry[i].message_id == message_id) {
			pthread_mutex_unlock(&lock_cb_entry);
			return &cb_entry[i];
		}
	}

	pthread_mutex_unlock(&lock_cb_entry);

	return NULL;
}

int callback_set_userdata(int message_id, void *ud, int ud_option)
{
	struct callback *cbe = callback_find(message_id);
	if (cbe == NULL) {
		log_err("do not find callback\n");
		return -1;
	}

	// first if ud != NULL
	if (cbe->ud != NULL) {
		int ret = callback_del_userdata(message_id);
	}

	pthread_mutex_lock(&lock_cb_entry);

	if (ud == NULL) {
		cbe->ud_option = -1;
	} else {
		cbe->ud_option = ud_option;
	}

	cbe->ud = ud;

	DEBUG("now message_id: %d set user data:%p\n", message_id, ud);

	pthread_mutex_unlock(&lock_cb_entry);

	return 0;
}

int callback_del_userdata(int message_id)
{
	struct callback *cbe = callback_find(message_id);
	if (cbe == NULL) {
		log_err("do not find callback\n");
		return -1;
	}
	
	pthread_mutex_lock(&lock_cb_entry);

	if (cbe->ud == NULL) {
		pthread_mutex_unlock(&lock_cb_entry);
		return -1;
	} else {
		if (cbe->ud_option == FREE_BY_OTHER) {
			cbe->ud = NULL;
		} else if (cbe->ud_option == FREE_BY_SELF) {
			free(cbe->ud);
			cbe->ud = NULL;	
		} else {
			log_err("ud_option error\n");
			pthread_mutex_unlock(&lock_cb_entry);
			return -1;
		}
	}

	pthread_mutex_unlock(&lock_cb_entry);

	return 0;
}

void callback_dump(void)
{
	int i;
	
	for (i = 0; i < n_cb_entry; i++) {
		DEBUG("callback_find:message_id:%d\n", cb_entry[i].message_id);
		struct callback *cbe = callback_find(cb_entry[i].message_id);	
		if (cbe == NULL) {
			log_err("do not find cb\n");
			return;
		}
		DEBUG("callback message_id = %d, ud = %p, cb = %p, ud_option = %d\n", 
						cbe->message_id, cbe->ud, cbe->cb, cbe->ud_option);
	}
}

void callback_destroy(void)
{
	assert(cb_entry);
	assert(n_cb_entry > 0);
	
	int i;
	for (i = 0; i < n_cb_entry; i++) {
		int message_id = cb_entry[i].message_id;
		callback_del_userdata(message_id);
	}

	free(cb_entry);
	cb_entry = NULL;
	n_cb_entry = -1;
}

int pb_message_pack(void *pb, struct protobuf_message *opt, char **msg, int *sz)
{
	if (pb == NULL || opt == NULL || msg == NULL || sz == NULL) {
		log_err("pb_message_pack param error\n");
		return -1;
	}

	*sz = opt->get_packed_size(pb);
	*msg = malloc(*sz);
	
	if (opt->pack(pb, *msg) < 0) {
		log_err("msg pack error\n");
		return -1;
	}
	return 0;
}

// CLIENT              SERVER
// READ_TIME
//   --------------------->
// READ_TIME_RESPONSE
//  <---------------------
CALLBACK(READ_TIME_RESPONSE)
{
	if (sz < 0 || msg == NULL) return -1;
	if (sc == NULL) return -1;

	DEBUG("[CALLBACK:%d] read time from server OK\n", header->message_id);
	struct protobuf_message *st_opt = protobuf_message_find("SynTime");
	assert(st_opt);

	SynTime *st_req = st_opt->unpack(NULL, sz, msg);
	if (st_req == NULL) {
		log_err("synctime unpack error\n");
		return -1;
	}			

	if(st_req->timestamp) {
		DEBUG("read server time clock:%s\n", st_req->timestamp);
	} else {
		log_err("server time return NULL\n");
		st_opt->free_unpacked(st_req, NULL);
		return -1;
	}

	st_opt->free_unpacked(st_req, NULL);
	
	return 0;
}

// CLIENT              SERVER
// READ_CONF
//   --------------------->
// READ_CONF_RESPONSE
//  <---------------------
CALLBACK(READ_CONF_RESPONSE)
{
	DEBUG("[CALLBACK:%d] read configure from server OK\n", header->message_id);
	return 0;
}

// CLIENT              SERVER
// UPDATE_TIME
//  <---------------------
// UPDATE_TIME_RESPONSE
//   --------------------->
CALLBACK(UPDATE_TIME)
{
	if (sz < 0 || msg == NULL) return -1;
	if (sc == NULL) return -1;

	struct protobuf_message *st_opt = protobuf_message_find("SynTime");
	assert(st_opt);

	SynTime *st_req = st_opt->unpack(NULL, sz, msg);
	if (st_req == NULL) {
		log_err("syntime unpack error\n");
		return -1;
	}

	if (st_req->timestamp == NULL) {
		log_err("sync timestamp == NULL\n");
		st_opt->free_unpacked(st_req, NULL);
		return -1;	
	}

	DEBUG("[CALLBACK:%d] received a update time message, set time:%s\n", header->message_id, st_req->timestamp);
	
	st_opt->free_unpacked(st_req, NULL);
	

	MessageHeader header_resp = MESSAGE_HEADER__INIT;
	message_header_copy(header, &header_resp);
	header_resp.message_id = MESSAGE_ID(UPDATE_TIME_RESPONSE);

	ResponseCode respcode_resp = RESPONSE_CODE__INIT;
	respcode_resp.code = RESPONSE__OK;
	respcode_resp.log = "Hello, Set Time Success";
		
	struct protobuf_message *r_opt = protobuf_message_find("ResponseCode");	
	assert(r_opt);

	char *msg_resp;
	int sz_resp;

	pb_message_pack(&respcode_resp, r_opt, &msg_resp, &sz_resp);
	send_message(sc, &header_resp, msg_resp, sz_resp);
	
	free(msg_resp);
	
	return 0;
}

// CLIENT              SERVER
// UPDATE_CONF
//  <---------------------
// UPDATE_CONF_RESPONSE
//   --------------------->
CALLBACK(UPDATE_CONF)
{
	DEBUG("[CALLBACK:%d] received a update configure message\n", header->message_id);
	// TODO header copy
	return 0;
}


// CLIENT              SERVER
// REBOOT
//  <---------------------
// REBOOT_RESPONSE
//   --------------------->
CALLBACK(REBOOT)
{
	DEBUG("[CALLBACK:%d] received a reboot message\n", header->message_id);
	// TODO header copy
	return 0;
}


// CLIENT              SERVER
// READ_CONTROLLER_STATE
//  <---------------------
// READ_CONTROLLER_STATE_RESPONSE
//   --------------------->
CALLBACK(READ_CONTROLLER_STATE)
{
	if (sz < 0 || msg == NULL) return -1;
	if (ud == NULL) return -1;
	if (sc == NULL) return -1;
	
	DEBUG("[CALLBACK:%d] received a read controller state message\n", header->message_id);
	struct room_model *rm = ud;
	struct protobuf_message *c_opt = protobuf_message_find("Controller");
	assert(c_opt);
	
	Controller *c_req = c_opt->unpack(NULL, sz, msg);
	if (c_req == NULL) {
		log_err("controller__unpack error\n");
		return -1;
	}

	struct controller *c_dev = room_model_find_controller(rm, c_req->controller_id);
	if (c_dev == NULL) {
		log_err("do not find this controller:%d\n", c_req->controller_id);
		return -1;
	}

	c_opt->free_unpacked(c_req, NULL);

	MessageHeader header_resp = MESSAGE_HEADER__INIT;
	message_header_copy(header, &header_resp);
	header_resp.message_id = MESSAGE__ID__READ_CONTROLLER_STATE_RESPONSE;

	DEBUG("c_dev status:%d\n", c_dev->state);

	Controller c_resp = CONTROLLER__INIT;
	c_resp.controller_id = c_dev->id;
	c_resp.has_state = 1;
	c_resp.state = c_dev->state;

	char *msg_resp;
	int sz_resp;
	pb_message_pack(&c_resp, c_opt, &msg_resp, &sz_resp);
	send_message(sc, &header_resp, msg_resp, sz_resp);

	free(msg_resp);

	return 0;
}

// CLIENT              SERVER
// UPDATE_CONTROLLER_STATE
//  <---------------------
// UPDATE_CONTROLLER_STATE_RESPONSE
//   --------------------->
CALLBACK(UPDATE_CONTROLLER_STATE)
{
	if (sz < 0 || msg == NULL) return -1;
	if (ud == NULL) return -1;
	if (sc == NULL) return -1;

	DEBUG("[CALLBACK:%d] received a update controller state message\n", header->message_id);

	struct room_model *rm = ud;
	struct protobuf_message *c_opt = protobuf_message_find("Controller");
	assert(c_opt);

	Controller *c_req = c_opt->unpack(NULL, sz, msg);
	if (c_req == NULL) {
		log_err("controller__unpack error\n");
		return -1;
	}

	struct controller *c_dev = room_model_find_controller(rm, c_req->controller_id);
	if (c_dev == NULL) {
		log_err("do not find this controller:%d\n", c_req->controller_id);
		return -1;
	}
		
	if (c_req->has_state == 0) {
		log_err("request do not has state param\n");
		c_opt->free_unpacked(c_req, NULL);
		return -1;
	}

	DEBUG("c_req:%d\n", c_req->state);
	// change state!!! here
	c_dev->state = c_req->state;
//	// TODO: send command to PLC!

	c_opt->free_unpacked(c_req, NULL);

	MessageHeader header_resp = MESSAGE_HEADER__INIT;
	message_header_copy(header, &header_resp);
	header_resp.message_id = MESSAGE_ID(UPDATE_CONTROLLER_STATE_RESPONSE);

	ResponseCode respcode_resp = RESPONSE_CODE__INIT;
	respcode_resp.code = RESPONSE__OK;
	respcode_resp.log = "Hello, Controller state has been set success";

	struct protobuf_message *r_opt = protobuf_message_find("ResponseCode");	
	assert(r_opt);
	
	char *msg_resp;
	int sz_resp;

	pb_message_pack(&respcode_resp, r_opt, &msg_resp, &sz_resp);
	send_message(sc, &header_resp, msg_resp, sz_resp);
	
	free(msg_resp);
	return 0;
}

// CLIENT              SERVER
// READ_SENSOR_DATA
//  <---------------------
// READ_SENSOR_DATA_RESPONSE
//   --------------------->
CALLBACK(READ_SENSOR_DATA)
{
	DEBUG("[CALLBACK:%d] received a read_sensor_data message\n", header->message_id);
//	Sensor *sreq = sensor__unpack(NULL, sz, msg);
//	if (sreq == NULL) {
//		log_err("unpack msg error\n");
//		return -1;
//	}


//	TODO: send request to thread_sensor, and recaculate sensor data
//	TODO: reconfirm this command return back a single sensor data or a whole room sensor data
	return 0;
}

// CLIENT              SERVER
// SENSOR_DATA_PUSH
//   --------------------->
// NO CALLBACK

struct ping_stats {
	int received;
};

CALLBACK(PING)
{
	if (sz < 0 || msg == NULL) return -1;
	if (sc == NULL) return -1;

	// if this is first time then set ud
	if (ud == NULL) {
		struct ping_stats *t = malloc(sizeof(struct ping_stats));
		assert(t);
		t->received = 0;
		callback_set_userdata(header->message_id, t, FREE_BY_SELF);
	}

	struct ping_stats *p_stats = ud;
	assert(p_stats);
	
	p_stats->received++;

	struct protobuf_message *ping_opt = protobuf_message_find("Ping");
	assert(ping_opt);

	Ping *ping_req = ping_opt->unpack(NULL, sz, msg);
	if (ping_req == NULL) {
		log_err("ping unpack error\n");
		return -1;
	}

	DEBUG("[CALLBACK:%d] received a ping message, stats:%d, src:%s, say:%s\n",
			 header->message_id, p_stats->received, ping_req->src, ping_req->echo);

	ping_opt->free_unpacked(ping_req, NULL);

	return 0;
}

// Helper Function
int send_read_time_request(struct socket_client *sc)
{
	DEBUG("sending a read time message to server\n");

	MessageHeader header = MESSAGE_HEADER__INIT;
	header.message_id = MESSAGE__ID__READ_TIME;
	header.has_type = 1;
	header.type = MESSAGE_HEADER__TYPE__REQUEST;

	if (send_message(sc, &header, NULL, 0) < 0) {
		DEBUG("send message error\n");
		log_err("send error\n");
		return -1;
	}
	return 0;
}

int send_room_sensor_data(struct socket_client *sc, struct room_model *rm, int room_id)
{	
	assert(sc);
	assert(rm);
	assert(room_id >= 0);

	DEBUG("sending room[%d] sensordata message to server\n", room_id);
	struct protobuf_message *sd_opt = protobuf_message_find("SensorData");
	assert(sd_opt);
	
	struct protobuf_message *s_opt = protobuf_message_find("Sensor");
	assert(s_opt);
	
	struct protobuf_message *t_opt = protobuf_message_find("SynTime");
	assert(t_opt);

	// pack rm[i] to sensor_data
	SensorData sd;
	sd_opt->init(&sd);

	MessageHeader header = MESSAGE_HEADER__INIT;
	header.message_id = MESSAGE__ID__SENSOR_DATA_PUSH;
	header.has_type = 1;
	header.type = MESSAGE_HEADER__TYPE__REQUEST;

	struct room *r = &rm->room[room_id];
	sd.room_id = r->id;
	sd.n_sensor = r->n_sensor;
	sd.sensor = malloc(sizeof(Sensor*)*sd.n_sensor);
	Sensor *t = malloc(sizeof(Sensor)*sd.n_sensor);
	assert(sd.sensor);
	assert(t);
	sd.time = malloc(sizeof(SynTime));
	assert(sd.time);	
	t_opt->init(sd.time);

	time_t ts;
	time(&ts);
	struct tm *tm = localtime(&ts);

	char tstamp[255];
	sprintf(tstamp, "%02d-%02d-%02d %02d:%02d:%02d", 
			tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	sd.time->timestamp = tstamp;

	int j;
	for (j = 0; j < r->n_sensor; j++) {			
		s_opt->init(&t[j]);	
		t[j].id = r->sensor[j].id;
		t[j].has_type = 1;
		t[j].type = r->sensor[j].type;
		t[j].has_value = 1;
		t[j].value = sensor_get_fuse(&r->sensor[j]);
		sd.sensor[j] = &t[j];
	}

	int sz;
	char *msg;
	pb_message_pack(&sd, sd_opt, &msg, &sz);
	if (send_message(sc, &header, msg, sz) < 0) {
		DEBUG("send error\n");
	}

	free(msg);
	free(sd.sensor);
	free(t);
	free(sd.time);

	return 0;
}

int send_init_configure(struct socket_client *sc, struct room_model *rm, struct config *config)
{
	assert(sc);
	assert(rm);
	assert(config);
	
	DEBUG("sending init configure message to server\n");
	struct protobuf_message *init_opt = protobuf_message_find("Init");
	if (init_opt == NULL) {
		log_err("protobuf message do not find.\n");
		return -1;
	}

	struct protobuf_message *roomconf_opt = protobuf_message_find("RoomConf");
	if (roomconf_opt == NULL) {
		log_err("protobuf message do not find.\n");
		return -1;
	}

	struct protobuf_message *sensor_opt = protobuf_message_find("Sensor");
	if (sensor_opt == NULL) {
		log_err("protobuf message do not find.\n");
		return -1;
	}

	struct protobuf_message *controller_opt = protobuf_message_find("Controller");
	if (controller_opt == NULL) {
		log_err("protobuf message do not find.\n");
		return -1;
	}

	Init init_msg;
	init_opt->init(&init_msg);
	init_msg.n_roomconf = rm->n_room;
	init_msg.roomconf = malloc(sizeof(RoomConf *)*rm->n_room);
	RoomConf *tr = malloc(sizeof(RoomConf)*rm->n_room);
	assert(init_msg.roomconf);
	assert(tr);
	
	struct room *room = rm->room;

	int i, j;
	for (i = 0; i < rm->n_room; i++) {
		// 1.1 init tr[i] and fill to init_msg.roomconf[i]
		roomconf_opt->init(&tr[i]);
		tr[i].id = room[i].id;
		tr[i].n_sensor = room[i].n_sensor;
		tr[i].sensor = malloc(sizeof(Sensor *)*tr[i].n_sensor);
		tr[i].n_controller = room[i].n_controller;
		tr[i].controller = malloc(sizeof(Controller *)*tr[i].n_controller);
		assert(tr[i].sensor);
		assert(tr[i].controller);
	
		// 1.1.1 roomconf->sensor
		Sensor *ts = malloc(sizeof(Sensor)*tr[i].n_sensor);
		assert(ts);

		for (j = 0; j < tr[i].n_sensor; j++) {
			struct sensor *s = &room[i].sensor[j];
			sensor_opt->init(&ts[j]);
			ts[j].id = s->id;
			ts[j].has_type = 1;
			ts[j].type = s->type;
			tr[i].sensor[j] = &ts[j];
		}
		
		// 1.1.2 roomconf->controller
		Controller *tc = malloc(sizeof(Controller)*tr[i].n_controller);
		assert(tc);

		for (j = 0; j < tr[i].n_controller; j++) {
			struct controller *c = &room[i].controller[j];
			controller_opt->init(&tc[j]);
			tc[j].controller_id = c->id;
			tc[j].has_type = 1;
			tc[j].type = c->type;	
			tr[i].controller[j] = &tc[j];
		}					

		init_msg.roomconf[i] = &tr[i];
	}

	MessageHeader header = MESSAGE_HEADER__INIT;
	header.message_id = MESSAGE__ID__INIT;
	header.has_type = 1;
	header.type = MESSAGE_HEADER__TYPE__REQUEST;

	int sz;
	char *msg;
	pb_message_pack(&init_msg, init_opt, &msg, &sz);
	send_message(sc, &header, msg, sz);
	
	free(msg);

	// 3. free something
	for (i = 0; i < rm->n_room; i++) {
		RoomConf *r = init_msg.roomconf[i];
		if (r->sensor[0]) free(r->sensor[0]);     r->sensor[0] = NULL;
		if (r->sensor)    free(r->sensor);        r->sensor = NULL;
		if (r->controller[0]) free(r->controller[0]); r->controller[0] = NULL;
		if (r->controller) free(r->controller);    r->controller = NULL;
	}
	
	if (init_msg.roomconf[0]) free(init_msg.roomconf[0]); init_msg.roomconf[0] = NULL;
	if (init_msg.roomconf) free(init_msg.roomconf);    init_msg.roomconf = NULL;	

	return 0;	
}
