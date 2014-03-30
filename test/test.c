#include "base.h"
#include "util.h"

int main(int argc, char *argv[])
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	int fd = open_socket_server(7890);

	while (1) {
		int client_fd = accept(fd, NULL, NULL);
		if (client_fd < 0) continue;


		for (;;) {
			char ch[] = "Hello, World\n";
			int ret = safe_write(client_fd, ch, sizeof(ch));
			if (ret < 0) break;
			usleep(100000);
		}
	}

	return 0;
}
