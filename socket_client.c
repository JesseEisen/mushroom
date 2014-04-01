#include "base.h"
#include "util.h"
#include "socket_client.h"
#include "cb.h"
#include "log.h"
#define MAX_PACKET_SIZE (1 << 16)
#define MAX_HEADER_SIZE (MAX_PACKET_SIZE - 2)

const char Prefix[] = "MUSHROOM";

struct packet {
	uint32_t pkt_sz:24;
	char *payload;
};

struct Message {
	MessageHeader *header;
	char *msg;
	uint32_t sz;
};

void thread_cleanup(void *arg)
{
	DEBUG("thread cancel. tid:%d\n", (unsigned int)pthread_self());
}

void *socket_recv_raw2pkt(void *arg)
{
	struct socket_client *sc = (struct socket_client *)arg;

	DEBUG("socket_recv_raw2pkt start success...\n");	

	pthread_cleanup_push(thread_cleanup, NULL);

RESTART:
	while (sc->is_available) {
		socket_client_wait_ready(sc);
		int i = 0;
		char ch;
		do {
			DEBUG("...\n");
			int ret = read_byte(sc->sock_fd, &sc->recv, &ch);
			if (ret < 0) {
				log_err("read err\n");
				//TODO sock_err LOCK!
				sc->sock_err = 1;
				goto RESTART;
			}
			if (ch != Prefix[i]) {
				goto RESTART;
			}
			i++;
		} while (i < sizeof(Prefix)-1);

		char sz_buf[3];
		for (i = 0; i < 3; i++) {
			int ret;
			ret = read_byte(sc->sock_fd, &sc->recv, &sz_buf[i]);
			if (ret < 0) {
				log_err("read err\n");
				sc->sock_err = 1;
				goto RESTART;
			}
		}

		uint32_t pkt_sz;
		int24_decode(sz_buf, &pkt_sz);

		// assert MTU
		if (pkt_sz < 0 || pkt_sz > MAX_PACKET_SIZE) {
			log_err("package sz error, pkt_sz = %d\n", pkt_sz);
			goto RESTART;
		}

		char *payload = malloc(pkt_sz);
		assert(payload);

		int ret = read_stream(sc->sock_fd, &sc->recv, payload, pkt_sz);
		if (ret < 0) {
			log_err("read err\n");
			sc->sock_err = 1;
			free(payload);
			goto RESTART;
		}

		struct packet *pkt_in = malloc(sizeof(struct packet));
		assert(pkt_in);

		pkt_in->pkt_sz = pkt_sz;
		pkt_in->payload = payload;

		char hexdump[8092];
		char *p = hexdump;

		memset(hexdump, 0, sizeof(hexdump));
		for (i = 0; i < pkt_in->pkt_sz; i++) {
			snprintf(p, 3, "%02x", payload[i]);
			p += 2;
		}

		//		log_info("%s", hexdump);
		DEBUG("recv message: MUSHROOM|%d|%s\n\n", pkt_sz, hexdump);

		put_data(sc->raw_in, (void *)pkt_in, 1 /* PRIORITY */);
		//DEBUG("put in address:%p\n", pkt_in);
	}

	pthread_cleanup_pop(0);
	return (void *)NULL;
}

void *socket_recv_pkt2msg(void *arg)
{
	assert(arg);

	struct socket_client *sc = (struct socket_client *)arg;
	DEBUG("socket_recv_pkt2msg start success...\n");	

	pthread_cleanup_push(thread_cleanup, NULL);
	while(sc->is_available) {
		DEBUG("...\n");
		struct packet *pkt_in = get_data(sc->raw_in, NULL);
		//DEBUG("get out address:%p\n", pkt_in);
		assert(pkt_in);

		uint32_t header_sz;
		int24_decode(pkt_in->payload, &header_sz);
		if (header_sz < 0 || header_sz > MAX_HEADER_SIZE) {
			log_err("header_sz error\n");
			goto CLEAN;
		}

		MessageHeader *header = message_header__unpack(NULL, header_sz, pkt_in->payload+3);
		if (header == NULL) {
			log_err("message header unpack error\n");
			goto CLEAN;
		}	

		struct Message *msg = malloc(sizeof(struct Message));
		assert(msg);

		msg->header = header;
		int sz = pkt_in->pkt_sz - 3 - header_sz;
		if (sz < 0) {
			log_err("msg sz error, sz:%d\n", sz);
			free(msg);
			message_header__free_unpacked(header, NULL);
			goto CLEAN;
		}		

		if (sz == 0) {
			msg->msg = NULL;
		} else {
			msg->msg = malloc(sz);
			assert(msg->msg);
			memcpy(msg->msg, pkt_in->payload+3+header_sz, sz);
		}

		msg->sz = sz;

		put_data(sc->msg_in, (void *)msg, 1/* PRIORITY */);
CLEAN:
		free(pkt_in->payload);
		free(pkt_in);
	}

	pthread_cleanup_pop(0);

	return (void *)NULL;
}

void *socket_send(void *arg)
{
	assert(arg);

	struct socket_client *sc = (struct socket_client *)arg;

	DEBUG("Thread socket_send start success..\n");

	struct packet *pkt_out;

RESTART:
	while (sc->is_available) {
		DEBUG("...\n");
		socket_client_wait_ready(sc);
		pkt_out = get_data(sc->raw_out, NULL);
		assert(pkt_out);

		int ret = safe_write(sc->sock_fd, (void *)Prefix, sizeof(Prefix)-1);
		if (ret < 0) {
			log_err("safe write error\n");
			sc->sock_err = 1;
			free(pkt_out->payload);
			free(pkt_out);
			goto RESTART;
		}

		char sz_buf[3];
		int24_encode(pkt_out->pkt_sz, sz_buf);
		//DEBUG("**********************send pkt_sz:%d, buf: [0]:%x, [1]:%x, [2]:%x************************************\n", pkt_out->pkt_sz, sz_buf[0], sz_buf[1], sz_buf[2]);	
		ret = safe_write(sc->sock_fd, sz_buf, 3);
		if (ret < 0) {
			log_err("safe write error\n");
			sc->sock_err = 1;
			free(pkt_out->payload);
			free(pkt_out);
			goto RESTART;
		}

		ret = safe_write(sc->sock_fd, pkt_out->payload, pkt_out->pkt_sz);
		if (ret < 0) {
			log_err("safe write error\n");
			sc->sock_err = 1;
			free(pkt_out->payload);
			free(pkt_out);
			goto RESTART;
		}


		char hexdump[8092];
		char *p = hexdump;

		memset(hexdump, 0, sizeof(hexdump));
		int i;
		for (i = 0; i < pkt_out->pkt_sz; i++) {
			snprintf(p, 3, "%02x", pkt_out->payload[i]);
			p += 2;
		}

		//		log_info("%s", hexdump);
		DEBUG("send message:MUSHROOM|%d|%s\n\n", pkt_out->pkt_sz, hexdump);

		free(pkt_out->payload);
		free(pkt_out);
	}

	return (void *)NULL;
}

void *socket_eating(void *arg)
{
	assert(arg);
	struct socket_client *sc = (struct socket_client *)arg;

	pthread_cleanup_push(thread_cleanup, NULL);

	while (sc->is_available) {
		DEBUG("...\n");
		struct Message *msg = get_data(sc->msg_in, NULL);
		assert(msg);

		MessageHeader *header = msg->header;		
		assert(header);	

		if (header->has_type && header->type == MESSAGE_HEADER__TYPE__TEXT) {
			// Just print Text
			DEBUG("received a text message, message_id:%d\n", header->message_id);
			int i;
			for (i = 0; i < msg->sz; i++) {
				printf("%c", msg->msg[i]);
			}

			printf("\n");
			goto CLEAN;
		}

		struct callback *cbe = callback_find(header->message_id);
		if (cbe == NULL) {
			log_err("message_id:%d, service not find.\n", header->message_id);
			goto CLEAN;
		}

		int ret = cbe->cb(sc, cbe->ud, header, msg->msg, msg->sz);
		if (ret < 0) {
			log_err("callback return error\n");
			goto CLEAN;
		}
CLEAN:
		if (msg->msg) free(msg->msg);
		msg->msg = NULL;

		free(msg);
		msg = NULL;

		message_header__free_unpacked(header, NULL);
		header = NULL;
	}

	pthread_cleanup_pop(0);

	return (void *)NULL;	
}

void *socket_watchdog(void *arg)
{
	struct socket_client *sc = (struct socket_client *)arg;

	int flag = 0;

	while (sc->is_available) {
		usleep(1000000);
		if (sc->sock_err) {
			if (flag == 0) {
				log_err("socket error, now retry\n");
				DEBUG("socket error, now retry\n");
				flag = 1;
			}
			if (sc->sock_fd > 0) {
				close(sc->sock_fd);
			}
			sc->sock_fd = -1;
			sc->sock_fd = open_socket_client(sc->ip, sc->port);
			if (sc->sock_fd < 0) {
			//	usleep(1000000); // delay some time
				continue;
			}
			sc->sock_err = 0;
			DEBUG("socket reconnect success...\n");
			log_info("socket reconnect success...\n");
			flag = 1;
		}
	}

	return (void *)NULL;
}

int socket_client_start(struct socket_client *sc, char *ip, int port, int n_eating)
{
	assert(sc);
	assert(ip);
	assert(port > 0);

	sprintf(sc->ip, "%s", ip);
	sc->port = port;	

	DEBUG("...\n");
	sc->sock_fd = open_socket_client(ip, port);	
	if (sc->sock_fd < 0) {
		sc->sock_err = 1;
	} else {
		sc->sock_err = 0;
	}	

#if defined(LOCAL_QUEUE)
	sc->raw_in = malloc(sizeof(ptable));
	assert(sc->raw_in);
	create(sc->raw_in);

	sc->raw_out = malloc(sizeof(ptable));
	assert(sc->raw_out);
	create(sc->raw_out);

	sc->msg_in = malloc(sizeof(ptable));
	assert(sc->msg_in);
	create(sc->msg_in);

	sc->msg_out = malloc(sizeof(ptable));
	assert(sc->msg_out);
	create(sc->msg_out);
#elif defined(POSIX_MQ_QUEUE)
	sc->raw_in = malloc(sizeof(ptable));
	assert(sc->raw_in);
	create(sc->raw_in, "/mushroomraw_in");

	sc->raw_out = malloc(sizeof(ptable));
	assert(sc->raw_out);
	create(sc->raw_out, "/mushroomraw_out");

	sc->msg_in = malloc(sizeof(ptable));
	assert(sc->msg_in);
	create(sc->msg_in, "/mushroommsg_in");

	sc->msg_out = malloc(sizeof(ptable));
	assert(sc->msg_out);
	create(sc->msg_out, "/mushroommsg_out");
#else
#error "You must define LOCAL_QUEUE or POSIX_MQ_QUEUE to choice a message queue\n"
#endif

	sc->is_available = 1;
	sc->sock_err = 0;

	memset(&sc->recv, 0, sizeof(struct recv));

	pthread_attr_t attr;

	pthread_attr_init(&attr);
	DEBUG("...\n");
	//	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	int ret = pthread_create(&sc->tid_recv_raw2pkt, &attr, socket_recv_raw2pkt, (void *)sc);
	assert(ret == 0);

	DEBUG("...\n");
	ret = pthread_create(&sc->tid_recv_pkt2msg, &attr, socket_recv_pkt2msg, (void *)sc);
	assert(ret == 0);

	DEBUG("...\n");
	ret = pthread_create(&sc->tid_send,         &attr, socket_send,         (void *)sc);
	assert(ret == 0);

	DEBUG("...\n");
	sc->n_eating = 1;
	sc->tid_eating = malloc(sizeof(pthread_t)*sc->n_eating);
	ret = pthread_create(&sc->tid_eating[0], &attr, socket_eating, (void *)sc);
	assert(ret == 0);

	// TODO: multi eating thread for test
	//	sc->n_eating = n_eating;
	//	int i;
	//	for (i = 0; i < n_eating; i++) {
	//		pthread_create(sc->tid_eating, &attr, socket_eating, (void *)sc);
	//	}

	DEBUG("...\n");
	ret = pthread_create(&sc->tid_watchdog,         &attr, socket_watchdog,         (void *)sc);
	assert(ret == 0);

	pthread_attr_destroy(&attr);	
	return 0;
}

int socket_client_stop (struct socket_client *sc)
{
	// 1. cancel all thread	
	assert(sc);
	sc->is_available = 0;

	pthread_cancel(sc->tid_recv_raw2pkt);
	pthread_cancel(sc->tid_recv_pkt2msg);
	pthread_cancel(sc->tid_send);
	pthread_cancel(sc->tid_watchdog);

	int i;
	for (i = 0; i < sc->n_eating; i++) {
		pthread_cancel(sc->tid_eating[i]);
	}

	pthread_join(sc->tid_recv_raw2pkt, NULL);
	pthread_join(sc->tid_recv_pkt2msg, NULL);
	pthread_join(sc->tid_send, NULL);
	for (i = 0; i < sc->n_eating; i++) {
		pthread_join(sc->tid_eating[i], NULL);
	}

	free(sc->tid_eating);

	pthread_join(sc->tid_watchdog, NULL);

	// 2. clean ptable
	// TODO: cleanup -> coredump
	cleanup(sc->raw_in);
	cleanup(sc->raw_out);
	cleanup(sc->msg_in);
	cleanup(sc->msg_out);

	// 3. close fd
	if (sc->sock_fd > 0)
		close(sc->sock_fd);

	sc->sock_err = 1;

	return 0;
}

int send_message(struct socket_client *sc, MessageHeader *header, void *msg, uint32_t sz)
{
	assert(sc);

	if (header == NULL) {
		log_err("message header error\n");
		return -1;
	}

	if (sc->sock_err) return -1;	

	// sz and msg check
	if (sz < 0) {
		log_err("sz error\n");
		return -1;
	} else if (sz == 0) {
		if (msg != NULL) {
			log_err("msg and sz do not match\n");
			return -1;
		}
	} else {
		if (msg == NULL) {
			log_err("msg and sz do not match\n");
			return -1;
		}
	}

	uint32_t header_sz = message_header__get_packed_size(header);

	if (header_sz < 0 || header_sz > MAX_HEADER_SIZE) {
		log_err("message header error\n");
		return -1;
	}

	uint32_t pkt_sz = 3 + header_sz + sz;		

	struct packet *pkt_out = malloc(sizeof(struct packet));
	assert(pkt_out);

	pkt_out->pkt_sz = pkt_sz;
	pkt_out->payload = malloc(pkt_out->pkt_sz);

	// len
	int24_encode(header_sz, pkt_out->payload);

	// header
	int ret = message_header__pack(header, pkt_out->payload+3);
	if (ret < 0) {
		log_err("message header pack error\n");
		free(pkt_out->payload);
		free(pkt_out);
		return -1;
	}

	// msg
	memcpy(pkt_out->payload+3+header_sz, msg, sz);

	put_data(sc->raw_out, (void *)pkt_out, 1);

	return 0;
}

int socket_client_wait_ready(struct socket_client *sc)
{
	while (sc->sock_err) {
		usleep(1000000);  // delay some time
	}
}
