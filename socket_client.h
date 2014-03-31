#ifndef _SOCKET_COMM_H_
#define _SOCKET_COMM_H_

#include "proto/message_header.pb-c.h"
#include "priqueue/queue.h"
#include "util.h"

struct socket_client {
	ptable *raw_in;
	ptable *raw_out;
	ptable *msg_in;
	ptable *msg_out;
	int sock_fd;

	char ip[255];
	int port;

	pthread_t tid_recv_raw2pkt;
	pthread_t tid_recv_pkt2msg;
	pthread_t tid_send;

	pthread_t *tid_eating;
	int n_eating;

	int is_available;

	struct recv recv;

	int sock_err;
	pthread_t tid_watchdog;
};

int socket_client_start(struct socket_client *sc, char *ip, int port, int n_eating);
int socket_client_stop (struct socket_client *sc);
int socket_client_wait_ready(struct socket_client *sc);
int send_message(struct socket_client *sc, MessageHeader *header, void *msg, uint32_t sz);

#endif
