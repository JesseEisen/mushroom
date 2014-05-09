#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int nwrite(int fd, char *buf, int sz)
{
	int actual = 0;

	if (fd < 0) return -1;

	while (sz > 0) {
		int n = write(fd, buf, sz);
		if (n < 0 && errno == EINTR) {
			continue;
		}

		if (n <= 0) {
			actual = -1;
			break;
		}

		sz  -= n;
		actual += n;
		buf += n;
	}

	return actual;
}

int nread(int fd, char *buf, int sz)
{
	int actual = 0;
	if (fd < 0) return -1;
	
	while (sz > 0) {
		int n = read(fd, buf, sz);
		if (n < 0 && errno == EINTR) {
			continue;
		}

		if (n <= 0) {
			actual = -1;
			break;
		}

		sz -= n;
		actual += n;
		buf += n;		
	}

	return actual;
}

int do_connect(char *ip, int port)
{
	int fd;
	struct sockaddr_in serv_addr;
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0) {
		goto _failed;
	}

	if(connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		goto _failed;
	}

	return fd;
_failed:
	close(fd);
	return -1;
}

int do_listen(const char * host, int port, int backlog) {
	// only support ipv4
	// todo: support ipv6 by getaddrinfo
	uint32_t addr = INADDR_ANY;
	if (host[0]) {
		addr = inet_addr(host);
	}
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		return -1;
	}
	int reuse = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(int))==-1) {
		goto _failed;
	}

	struct sockaddr_in my_addr;
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = addr;
	if (bind(listen_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		goto _failed;
	}
	if (listen(listen_fd, backlog) == -1) {
		goto _failed;
	}
	return listen_fd;
_failed:
	close(listen_fd);
	return -1;
}


int parse_endpoint(char *endpoint, char *ip, int *port)
{
	char *t;
	char *delim = strchr(endpoint, ':');
	if (!delim) {
		return -1;
	}

	t = endpoint;
	while (t != delim)
		*ip++ = *t++;
	*ip = '\0';

	t++;
	char buf[16];
	char *q = buf;
	while (*t != '\0')
		*q++ = *t++;
	*q = '\0';
	t++;

	*port = atoi(buf);

	return 0;
}
