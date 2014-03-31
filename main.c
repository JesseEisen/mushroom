#include "base.h"
#include "log.h"
#include "config.h"
#include "room.h"
#include "socket_client.h"
#include "cb.h"
#include "proto/message_header.pb-c.h"
#include "proto/service.pb-c.h"
#include "util.h"
#include "serial.h"

static int restart = 0;

int readCmdLine(struct socket_client *sc, struct room_model *rm, struct config *config)
{
	int nread;
	char buf[255];

	int cmd_fifo = open("./cmd.fifo", O_RDONLY);
	if (cmd_fifo < 0) {
		DEBUG("open cmd fifo error:%s\n", strerror(errno));
		return -1;
	}

	while(1) {
		memset(buf, 0, sizeof(buf));
		nread = read(cmd_fifo, buf, 255);

		if (nread > 0) {
			//write(STDOUT_FILENO, buf, nread);
			if (strncmp(buf, "send", 4) == 0) {

				if (strncmp(buf+5, "time", 4) == 0) {
					send_read_time_request(sc);
				} else if (strncmp(buf+5, "init", 4) == 0) {
					send_init_configure(sc, rm, config);		
				} else if (strncmp(buf+5, "sensordata", 10) == 0) {
					send_room_sensor_data(sc, rm, 0);
					send_room_sensor_data(sc, rm, 1);
					send_room_sensor_data(sc, rm, 2);
				}
			} else if (strncmp(buf, "dump", 4) == 0) {
				if (strncmp(buf+5, "config", 6) == 0) {
					config_dump(config);
				} else if (strncmp(buf+5, "room", 4) == 0) {
					room_model_dump(rm);
				} else if (strncmp(buf+5, "callback", 8) == 0) {
					callback_dump();
				} else if (strncmp(buf+5, "pb", 2) == 0) {
					protobuf_message_dump();
				}
			} else if (strcmp(buf, "quit\n") == 0) {
				restart = 0;
				break;
			} else if (strcmp(buf, "exit\n") == 0) {
				restart = 0;
				break;
			} else if (strcmp(buf, "restart\n") == 0) {
				DEBUG("system restart now!\n");
				sleep(1);
				restart = 1;
				break;
			}
		} else if (nread == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			} else {
				DEBUG("read error\n");
				// TODO;
			}
		} else if (nread == 0) {
			//			DEBUG("fd closed\n");
		}		

		usleep(200000);
	}

	close(cmd_fifo);

	return 0;
}

int test_log()
{
	log_info("mushroom system start\n");
	log_warning("mushroom system start\n");
	log_trace("mushroom system start\n");
	log_err("mushroom system start\n");

	log_info("hello, test log_info\n");
	log_warning("hello, test log_warning\n");
	log_trace("hello, test log_trace\n");
	log_err("hello, test log_err\n");

	DEBUG("log test passed......\n");
	return 0;
}

int test_config(struct config *config)
{
	config_dump(config);

	DEBUG("config test passed......\n");
	return 0;	
}

int test_room_model (struct room_model *rm)
{
	room_model_dump(rm);

	DEBUG("room_model test passed......\n");
}

int test_socket_client(struct socket_client *sc)
{
	//	MessageHeader header = MESSAGE_HEADER__INIT;
	//
	//	header.message_id = i++;
	//	int sz = message_header__get_packed_size(&header);
	//	DEBUG("sz:%d\n", sz);

	//	int i = 0, j = 0;
	//	while (1) {
	//		j++;
	//		if (j == 10) break;
	//		header.message_id = i++;
	//		if (send_message(sc, &header, NULL, 0) < 0) {
	//			DEBUG("send error\n");
	//		}
	//		usleep(100000);
	//	}

	DEBUG("socket client test passed......\n");

	return 0;
}

int test_callback()
{
	callback_dump();
	DEBUG("callback test passed......\n");

	return 0;
}

int test_protobuf_message()
{
	protobuf_message_dump();
	DEBUG("protobuf message test passed......\n");

	return 0;
}

struct sensor_data_push {
	struct socket_client *sc;
	struct room_model *rm;
	int sensor_data_push_freq;
};

void *thread_sensor_data_push(void *arg)
{
	struct sensor_data_push *sa = (struct sensor_data_push *)arg;

	struct room_model *rm = sa->rm;
	struct socket_client *sc = sa->sc;

	DEBUG("n_room:%d\n", rm->n_room);

	while (1) {
		int i, j;
		for (i = 0; i < rm->n_room; i++) {
			send_room_sensor_data(sc, rm, i);
		}

		sleep(sa->sensor_data_push_freq);
	}	

	return (void *)NULL;
}

//int sensor_push_start(struct socket_client *sc, struct room_model *rm, struct config *config)
int sensor_push_start(struct sensor_data_push *sdp)
{
	pthread_t tid_sensor_push;

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_create(&tid_sensor_push, &attr, thread_sensor_data_push, (void *)sdp);

	pthread_attr_destroy(&attr);

	return 0;
}

int main(int argc, char *argv[])
{
	signal(SIGPIPE, SIG_IGN); // fd err
	signal(SIGALRM, SIG_IGN); // usleep

RESTART:
	// system init
	log_init("log/info.log", "log/warning.log", "log/trace.log", "log/err.log");
	test_log();
	callback_init();	
	test_callback();

	protobuf_message_init();
	test_protobuf_message();

	struct config config;
	assert(config_load("mr.ini", &config) == 0);
	test_config(&config);

	struct room_model room_model;
	assert(room_model_load(&room_model, "mushroom.db") == 0);	
	test_room_model(&room_model);

	// set ud
	callback_set_userdata(MESSAGE__ID__READ_CONTROLLER_STATE, &room_model, FREE_BY_OTHER);
	callback_set_userdata(MESSAGE__ID__UPDATE_CONTROLLER_STATE, &room_model, FREE_BY_OTHER);

	struct callback *cb1 = callback_find(MESSAGE__ID__READ_CONTROLLER_STATE);
	struct callback *cb2 = callback_find(MESSAGE__ID__UPDATE_CONTROLLER_STATE);

	printf("ud_1:%p, ud_2:%p\n", cb1->ud, cb2->ud);

	struct serial serial;
	serial_start(&serial, &room_model, &config);

	struct socket_client socket_client;
#define SOCKET_CLIENT_EATING_THREAD_NUM 1
	socket_client_start(&socket_client, 
			config.remote_ip, 
			config.remote_port, 
			SOCKET_CLIENT_EATING_THREAD_NUM);	
	test_socket_client(&socket_client);

	// 1. send init message
	send_init_configure(&socket_client, &room_model, &config);

	// 2. send read_time
	send_read_time_request(&socket_client);

	// 3. start sensor_push
	struct sensor_data_push sdp;
	sdp.sc = &socket_client;
	sdp.rm = &room_model;
	sdp.sensor_data_push_freq = config.sensor_data_push_freq;
	sensor_push_start(&sdp);

	readCmdLine(&socket_client, &room_model, &config);
	if (restart == 1) {
		restart = 0;
		goto RESTART;
	}

	serial_stop(&serial);
	socket_client_stop(&socket_client);
	room_model_destroy(&room_model);
	callback_destroy();
	protobuf_message_destroy();
	log_destroy();

	DEBUG("byebye!\n");

	return 0;	
}
