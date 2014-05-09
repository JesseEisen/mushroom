#include "mrsh.h"
#include "console.h"
#include "util.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <sys/socket.h>

#define BACKLOG 32

static int sock_fd = -1;

static int _process_request(int fd)
{
	log_info("Hi~\n");
	return -1;	
}

static void *_thread_working(void *arg)
{
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sock_fd, &rfds);
	int max_fd = sock_fd;
	while (1) {
		fd_set tmpfds = rfds;
		int ret = select(max_fd+1, &tmpfds, NULL, NULL, NULL);
		if (ret < 0) {
			fprintf(stderr, "select error\n");
			continue;
		} else if (ret == 0) {
			// never happen
			continue;
		} else {
			int i;
			for (i = 0; i < max_fd+1; i++) {
				if (FD_ISSET(i, &tmpfds)) {
					if (i == sock_fd) {
						int conn_fd = accept(sock_fd, NULL, NULL);
						if (conn_fd < 0) {
							fprintf(stderr, "select error\n");
							continue;
						}
	
						max_fd = max_fd > conn_fd?max_fd:conn_fd;
						FD_SET(conn_fd, &rfds);
					} else {
						int ret = _process_request(i);
						if (ret < 0) {
							close(i);
							FD_CLR(i, &rfds);
							if (i == max_fd) {
								while (FD_ISSET(max_fd, &rfds)) max_fd -= 1;
							}
						}
					}
				}
			}
		}
	}

	return (void *)NULL;
}

void console_init(char *endpoint)
{
//	printf("%s", dump_config());
	char ip[255];
	int port;
	
	int ret = parse_endpoint(endpoint, ip, &port);
	if (ret < 0) {
		return;
	}
	sock_fd = do_listen(ip, port, BACKLOG);
	if (sock_fd < 0) {
		return;
	}

	pthread_t tid_console;
	ret = pthread_create(&tid_console, NULL, _thread_working, (void *)NULL);
	assert(ret == 0);
}

int console_test()
{
	if (sock_fd < 0) return -1;
	return 0;
}
