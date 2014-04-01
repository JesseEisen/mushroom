#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#define BUFSIZE 255

int open_serial_source(char *device)
{
	struct termios newtio;
	int fd;

	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY/* | O_NONBLOCK*/);
	if (fd < 0) {
		printf("serial open error:%s\n", strerror(errno));
		return -1;
	}

	/* Serial port setting */
	memset(&newtio, 0, sizeof(newtio));
	tcgetattr(fd, &newtio);
	newtio.c_cflag = CLOCAL | CREAD;
	newtio.c_cflag |= PARENB;
	newtio.c_cflag &= ~PARODD;
	newtio.c_cflag &= ~CSTOPB;
	newtio.c_cflag &= ~CSIZE;
	newtio.c_cflag |= CS7;

	newtio.c_iflag |= (INPCK | ISTRIP);
	//newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//newtio.c_iflag = IGNPAR | IGNBRK;
	cfsetispeed(&newtio, B9600);
	cfsetospeed(&newtio, B9600);

	/* Raw output_file */
	newtio.c_oflag = 0;

	if (tcflush(fd, TCIFLUSH) >= 0 && tcsetattr(fd, TCSANOW, &newtio) >= 0) {
		printf("fd ok\n");
		return fd;
	}

	close(fd);

	return -1;
}

int source_read(int fd, char *buf, int count)
{
	fd_set fds;
	int cnt;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	cnt = select(fd+1, &fds, NULL, NULL, NULL);
	if (cnt < 0)
		return -1;

	cnt = read(fd, buf, count);
	return cnt;
}

int source_write(int fd, const char *buf, int count)
{
	int actual = 0;
	/*	if (fcntl(fd, F_SETFL, 0) < 0) {
		return -1;
		}
	 */	
	//	while (count > 0) {
	int n = write(fd, buf, count);
	//		if (n < 0 && errno == EINTR) {
	//			continue;
	//		}
	//		if (n < 0) {
	//			actual = -1;
	//			break;
	//		}

	//		count -= n;
	//		actual += n;
	//		buf += n;
	//	}

	/*	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
	// WE ARE IN TROUBLE
	}
	 */
	//	return actual;
	return n;
}

#include "priqueue/queue.h"

typedef int (*ResponseCallback)(void *context, char *buf, int sz);

#define READ  1
#define WRITE 0

struct command {
	unsigned char buf[BUFSIZE];
	int sz;
	void *context;
	ResponseCallback respCallback;
};

char getAscii(int i) 
{
	if (i >=0 && i <= 9) return i+'0';
	else if (i>=10 && i <=15) return (i-10)+'A';
}

char getAddressAscii(int address, char buf[4])
{
	int x = address * 2 + 0x1000;

	int i, j, m, n;  
	i = x / (16 * 16 * 16); 
	j = (x / (16 * 16)) % 16; 
	m = x - i*(16*16*16) - j*(16*16); 
	m = m / 16; 
	n = x % 16; 

	buf[0] = getAscii(i);
	buf[1] = getAscii(j);
	buf[2] = getAscii(m);
	buf[3] = getAscii(n);
}

int getReadCommandFrame (char *buf, int *sz, int address, int num)
{
	if (buf == NULL || sz == NULL ||  
			(address < 0 || address > 255) || num < 0)  
		return -1; 

	buf[0] = 0x02;
	buf[1] = '0';

	getAddressAscii(address, &buf[2]);

	buf[6] = getAscii(num / 10);
	buf[7] = getAscii(num - num/10);

	buf[8] = 0x03;


	int i, sum = 0;
	for (i = 1; i <= 8; i++)
		sum += buf[i];

	i = sum&0xFF;
	buf[9]  = getAscii(i/16);
	buf[10] = getAscii(i%16);

	*sz = 11;

	for (i = 0; i < *sz; i++) {
		printf("%02x ", buf[i]);
	}

	return 0;
}

int getWriteCommandFrame(char *buf, int *sz, int address, int num, char *data)
{
	if (buf == NULL || sz == NULL ||  
			(address < 0 || address > 255) || num < 0)  
		return -1; 

	buf[0] = 0x02;
	buf[1] = '1';

	getAddressAscii(address, &buf[2]);

	buf[6] = getAscii(num / 10);
	buf[7] = getAscii(num - num/10);

	int i;
	for (i = 0; i < num*2; i+=4) {
		buf[8+i]   = data[i+2];
		buf[8+i+1] = data[i+3];
		buf[8+i+2] = data[i];
		buf[8+i+3] = data[i+1];
	}

	buf[8+num*2] = 0x03;

	int sum = 0;
	for (i = 1; i <= 8+num*2; i++)
		sum += buf[i];

	i = sum&0xFF;
	buf[8+num*2+1]  = getAscii(i/16);
	buf[8+num*2+2]  = getAscii(i%16);

	*sz = 8+num*2+3;

	for (i = 0; i < *sz; i++) {
		printf("%02x ", buf[i]);
	}

	printf("\n");

	return 0;
}

int test_plc_callback(void *context, char *buf, int sz)
{
	buf[sz] = '\0';

	int i;
	for (i = 0; i < sz; i++) {
		printf("%02x ", buf[i]);
	}

	printf("\n");

	//printf("A Message Received: %s\n\n\n", buf);

	return 0;
}

void *test_plc(void *param)
{
	ptable *table = (ptable *)param;


	while (1) {
		//		getWriteCommandFrame(cmd->buf, &cmd->sz, 123, 4, "1234ABCD");
		struct command *cmd = malloc(sizeof(struct command));
		getReadCommandFrame(cmd->buf, &cmd->sz, 123, 5);
		//		sprintf(cmd->buf, 
		//			"\\\x%02X \\\x%02X \\\x%02X \\\x%02X \\\x%02X \\\x%02X \\\x%02X \\\x%02X \\\x%02X \\\x%02X \\\x%02X",
		//			0x02, 0x30, 0x31, 0x30, 0x46, 0x36, 
		//				0x30, 0x34, 0x03, 0x37, 0x34);

		//		printf("%s\n", cmd->buf);
		//		cmd->sz = strlen(cmd->buf);

		cmd->context = NULL;
		cmd->respCallback = test_plc_callback;

#define PRIORITY 1
		put_data(table, (void *)cmd, PRIORITY);
		sleep(1);

	}
	return (void *)NULL;
}

//int main(int argc, char *argv[])
//int test(int argc, char *argv[])
//{
//	int fd = open_serial_source("/dev/ttyUSB0");
//	if (fd < 0) {
//		printf("serial open error\n");
//		return -1;
//	}
//
//	unsigned char buf[BUFSIZE];
//
//	// init a message queue i will use it in thread
//	ptable *p = malloc(sizeof(ptable));
////	create(p, "/test_serial");
//	create(p);
//
//	pthread_t plc_cmd_producer;
//	pthread_create(&plc_cmd_producer, NULL, test_plc, (void *)p);
//		
//	struct command *cmd;
//	int priority;
//	int cnt;
//
//	while (1) {
//			// 1. read message from message_queue
//			cmd = get_data(p, NULL);
//			// 2. write command to PLC serial
//			cnt = source_write(fd, cmd->buf, cmd->sz);
//			printf("cnt = %d\n", cnt);
//			// 3. read ACK from PLC serial and parse it (switch case)
//			cnt = source_read(fd, buf, BUFSIZE);
//			printf("cnt2 = %d\n", cnt);
//
//			// 4. parse buf
//			if (cnt < 0) {
//				printf("error\n");
//				// return -1;
//				continue;
//			}
//
//			cmd->respCallback(cmd->context, buf, cnt);
//			free(cmd);
//			cmd = NULL;
//
//			// next!
//	}
//
//	pthread_join(plc_cmd_producer, NULL);
//	cleanup(p);
//	return 0;
//}

#include "util.h"
#include "room.h"
#include "config.h"
#include "serial.h"

void *thread_serial_sensor(void *arg)
{
	struct serial *s = (struct serial*)arg;

	struct room_model *rm = s->rm;

	while (s->is_available) {
		// rand value
		DEBUG("...\n");
		int i, j;
		for (i = 0; i < rm->n_room; i++) {
			struct room *r = &rm->room[i];
			for (j = 0; j < r->n_sensor; j++) {
				int rand_val = get_rand(0, 255);
				sensor_add_value(&r->sensor[j], rand_val);
			}
		}		

		//DEBUG("now sleep %d s\n", s->read_plc_freq);
		sleep(s->read_plc_freq);

		// TODO: read from real serial
	}

	return (void *)NULL;
}


int serial_start(struct serial *s, struct room_model *rm, struct config *config)
{
	assert(s);
	assert(rm);
	assert(config);

	memset(s, 0, sizeof(struct serial));
	DEBUG("...\n");

	// TODO:
	s->serial_fd = -1;
	sprintf(s->dev, "%s", config->serial_path);
	s->is_available = 1;
	s->read_plc_freq = config->read_plc_freq;	
	s->rm = rm;

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	int ret = pthread_create(&s->tid_serial_sensor, &attr, thread_serial_sensor, (void *)s);
	DEBUG("...\n");
	assert(ret == 0);
	pthread_attr_destroy(&attr);

	return 0;
}

int serial_stop(struct serial *s)
{
	assert(s);

	s->is_available = 0;
	s->rm = NULL;
	s->read_plc_freq = -1;
	s->serial_fd = -1;

	pthread_cancel(s->tid_serial_sensor);
	pthread_join(s->tid_serial_sensor, NULL);

	return 0;
}
