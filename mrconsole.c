#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	int nread;
	char buf[255];

	int cmd_fifo = open("cmd.fifo", O_RDWR);
	if (cmd_fifo < 0) {
		printf("open cmd fifo error:%s\n", strerror(errno));
		return -1;
	}

	while(1) {
		printf("$ Welcome to mushroom console. CopyRight@cslg n6-412 Lab.\n");
		printf("$ CMD AS FOLLOWS\n");
		printf("$ send [time|init|sensordata]    -- send message to server\n"
				"$ dump [config|room|callback|pb] -- dump system current stats\n"
				"$ restart                        -- restart system\n"
				"$ quit                           -- quit\n"
				"$ "
		      );
		fflush(stdout);
		memset(buf, 0, sizeof(buf));
		nread = read(STDIN_FILENO, buf, 255);
		write(cmd_fifo, buf, nread);
		/*		if (strcmp(buf, "quit\n") == 0) {
				printf("byebye!\n");
				break;
				}
		 */
	}

	return 0;
}
