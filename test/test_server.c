#include "../base.h"
#include "../util.h"
#include "../proto/message_header.pb-c.h"
#include "../proto/service.pb-c.h"

char Prefix[] = "MUSHROOM";

#define MAX_PACKET_SIZE 1 << 16

int main(int argc, char *argv[])
{

	signal(SIGPIPE, SIG_IGN);

	int fd = open_socket_server(9999);
	char buf[1024];	


	while (1) {
RESTART:
		printf("now accept.\n");
		int client_fd = accept(fd, NULL, NULL);
		if (client_fd <= 0) {
			printf("Hello World\n");
			continue;
		}
//		if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0) {
//			printf("fcntl error\n");
//			continue;
//		}
		for (;;) {
//			int nread = read(client_fd, buf, 1024);
//			if (nread < 0) {
//				fprintf(stderr, "socket err\n");
//				break;
//			} else if (nread == 0) {
//				printf("received 0 byte, continue.\n");
//				close(client_fd);
//				break;
//			}
//
//			printf("received socket len:%d\n", nread);
//
//			int i;
//			for (i = 0; i < nread; i++) {
//				printf("%x ", buf[i]);
//			}
//
//			printf("\n");

		int i = 0;
		char ch;
		struct recv recv;
		do {
			int ret = read_byte(client_fd, &recv, &ch);
			if (ret < 0) {
				DEBUG("calling\n");
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
			ret = read_byte(client_fd, &recv, &sz_buf[i]);
			if (ret < 0) {
				DEBUG("calling\n");
				goto RESTART;
			}
		}

		uint32_t pkt_sz;
		uint32_t header_sz;
		int24_decode(sz_buf, &pkt_sz);
			
		// assert MTU
		if (pkt_sz < 0 || pkt_sz > MAX_PACKET_SIZE) {
			DEBUG("package sz error, pkt_sz = %d\n", pkt_sz);
			goto RESTART;
		}

		char *payload = malloc(pkt_sz);
		assert(payload);
		
		int ret = read_stream(client_fd, &recv, payload, pkt_sz);

		if (ret < 0) {
			DEBUG("calling\n");
			free(payload);
			goto RESTART;
		}

		int24_decode(payload, &header_sz);

		printf("pkt_sz:%d, header_sz:%d\n", pkt_sz, header_sz);

		MessageHeader *header = message_header__unpack(NULL, header_sz, payload+3);
		if (header == NULL) {
			printf("unpack error.\n");
			continue;
		}
			
		printf("message_id:%d\n", header->message_id);
		message_header__free_unpacked(header, NULL);

		int sz = pkt_sz - header_sz - 3;
		void *msg = payload+3+header_sz;
		Init *init_msg = init__unpack(NULL, sz, msg);

		if (init_msg == NULL) {
			printf("init msg unpack error\n");
			continue;
		}

		printf("n_sensor:%d, n_controller:%d\n", init_msg->n_roomconf, init_msg->n_config);

		int j;
		for (i = 0; i < init_msg->n_roomconf; i++) {
			RoomConf *r = init_msg->roomconf[i];
			printf("roomid:%d\n", r->id);
			for (j = 0; j < r->n_sensor; j++) {
				printf("sensor_id:%d, sensor_type:%d\n", r->sensor[j]->id, r->sensor[j]->type);
			}
		}

		init__free_unpacked(init_msg, NULL);

		printf("fd:%d\n", client_fd);

		MessageHeader header_r = MESSAGE_HEADER__INIT;
		header_r.message_id = 1024;
		header_r.has_type = 1;
		header_r.type = MESSAGE_HEADER__TYPE__TEXT;

		int p_len, h_len;

		char helloworld[] = "Hello World";
		h_len = message_header__get_packed_size(&header_r);
		p_len = 3 + h_len + sizeof(helloworld);

		// 1. prefix
		char Prefix[] = "MUSHROOM";
		ret = safe_write(client_fd, Prefix, sizeof(Prefix)-1);
		if (ret < 0) goto SocketErr;
		
		// p_len
		int24_encode(p_len, sz_buf);
		ret = safe_write(client_fd, sz_buf, 3);
		if (ret < 0) goto SocketErr;
			
		// h_len
		int24_encode(h_len, sz_buf);
		ret = safe_write(client_fd, sz_buf, 3);		
		if (ret < 0) goto SocketErr;

		// header
		char *hheader = malloc(h_len);
		message_header__pack(&header_r, hheader);
		
		ret = safe_write(client_fd, hheader, h_len);
		if (ret < 0) goto SocketErr;
		free(hheader);

		// data
		ret = safe_write(client_fd, helloworld, sizeof(helloworld));
		if (ret < 0) goto SocketErr;
			
		usleep(100000);
		continue;
SocketErr:
		close(client_fd);
		sleep(1);
		break;

		}
	}
//
	return 0;
}
