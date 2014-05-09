#include "serial.h"
#include "queue.h"
#include "log.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <pthread.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

struct _serial_stats {
	int n_send;
	int n_recv;
	int n_err;	
};

static char _device[255] = {0};
static int _fd = -1;
static struct queue *_Q = NULL;
static struct _serial_stats *_S = NULL;
static int MAGIC_NO = 0;

static int _check_command(char *buf, int sz)
{
	// TODO: implementation totally check

	int ret = 0;

	ret |= (buf[0] == 0x02);
	ret |= (buf[1] == 0x30 || buf[1] == 0x31);

	return ret;
}

static void *_thread_working(void *arg)
{
	int fd = _fd;
	while (1) {
_failed:
		usleep(1000);
		struct serial_request *req = queue_pop(_Q, NULL);

		if (_check_command(req->buf, req->sz) == 0) {
			log_err("cmd error\n");
			free(req);
			goto _failed;
		}

		int num = 0;
		if (req->buf[1] == 0x30) {
			// DATA size + STX(1 byte) + ETX(1 byte) + SUM(2 byte)
			num = ((req->buf[6]-'0')*10 + (req->buf[7]-'0'))*2+4;
		} else {
			num = 1;
		}

		if (num > 64*2+4) {
			log_err("cmd error\n");
			free(req);
			goto _failed;
		}

		// write command
		int ret = nwrite(fd, req->buf, req->sz);
		if (ret < 0) {
			log_err("serial error\n");
			free(req);
			goto _failed;
		}

		_S->n_send++;
		
		char resp[4096];
		memset(resp, 0, sizeof(resp));

		for (;;) {
			struct timeval tv;
			memset(&tv, 0, sizeof(struct timeval));
			tv.tv_sec = 5;
			tv.tv_usec = 0;	

			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(fd, &rfds);
	
			ret = select(fd+1, &rfds, NULL, NULL, &tv);
			if (ret == -1) {
				log_err("select error\n");
				free(req);
				goto _failed;
			} else if (ret == 0) {
				log_err("time expired\n");
				free(req);
				goto _failed;
			} else {
				int cnt = nread(fd, resp, num);
				if (cnt == -1) {
					log_err("serial error\n");
					free(req);
					goto _failed;
				}

				// call cb
				_S->n_recv++;
				req->cb(req->ud, resp, num);
				break;
			}
		}
	
		// free something
		free(req);
	}

	return (void *)NULL;
}




static int _set_serial_attr(int fd, int baude, char bits, char parity, char stop)
{
	struct termios options;
	if (tcgetattr(fd, &options) != 0) {
		return -1;
	}

	bzero(&options, sizeof(struct termios));

	switch (baude) {
		case 9600:
			cfsetispeed(&options, B9600);
			cfsetospeed(&options, B9600);
			break;
		default:
			return -1;
	}

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);     /*Input */
	options.c_oflag &= ~OPOST;      /*Output */

	options.c_cflag |= CLOCAL;
	options.c_cflag |= CREAD;

	switch (parity) {
		case 'N':
		case 'n':
		case 'S':
		case 's':
			options.c_cflag &= ~PARENB;
			break;
		case 'E':
		case 'e':
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			break;
		case 'O':
		case 'o':
			options.c_cflag |= PARENB;
			options.c_cflag |= PARODD;
			break;
		default:
			return -1;
	}

	if (options.c_cflag & PARENB) {
		// Enable Parity
		options.c_iflag |= (INPCK | ISTRIP);
	}

	switch (stop) {
		case '1':
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			return -1;
	}

	switch (bits) {
		case '7':
			options.c_cflag &= ~CSIZE;
			options.c_cflag |= CS7;
			break;
		case '8':
			options.c_cflag &= ~CSIZE;
			options.c_cflag |= CS8;
			break;
		default:
			return -1;
	}

	options.c_cc[VTIME] = 0;   /* 设置超时15 seconds */
	options.c_cc[VMIN] = 1;    /* define the minimum bytes data to be readed */
	tcflush(fd, TCIFLUSH);

	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		return -1;
	}

	return 0;        
}

void serial_init(char *dev)
{
	strncpy(_device, dev, 255);
	_fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
        if (_fd == -1) {
                fprintf(stderr, "[serial] %s\n", strerror(errno));
                return;
        }
	
	// default is '7E1'
	int ret = _set_serial_attr(_fd, 9600, '7', 'E', '1');
	if (ret < 0) {
		fprintf(stderr, "[serial] set serial attribute error\n");
		return;
	}
	
	_Q = queue_create();	
	assert(_Q);

	_S = malloc(sizeof(struct _serial_stats));
	assert(_S);
	memset(_S, 0, sizeof(*_S));

	pthread_t tid_serial;
	ret = pthread_create(&tid_serial, NULL, _thread_working, (void *)NULL);
	assert(ret == 0);

	MAGIC_NO = 0x1234;
}

int serial_test()
{
	if (MAGIC_NO != 0x1234) {
		return -1;
	}
	return 0;
}

int serial_send(struct serial_request *req)
{
	struct serial_request *req_copy = malloc(sizeof(struct serial_request));
	assert(req_copy);
	
	req_copy->ud = req->ud;
	req_copy->cb = req->cb;
	req_copy->sz = req->sz;
	memcpy(req_copy->buf, req->buf, sizeof(req->buf));
	
	queue_push(_Q, (void *)req_copy, /*priority*/1);
	return 0;
}

// helper function
static char _getAscii(int i)
{
	if (i >=0 && i <= 9) return i+'0';
	else if (i>=10 && i <=15) return (i-10)+'A';

	return -1;
}

static void _getAddressAscii(int address, char buf[4])
{
	int x = address * 2 + 0x1000;

	int i, j, m, n;
	i = x / (16 * 16 * 16);
	j = (x / (16 * 16)) % 16;
	m = x - i*(16*16*16) - j*(16*16);
	m = m / 16;
	n = x % 16;

	buf[0] = _getAscii(i);
	buf[1] = _getAscii(j);
	buf[2] = _getAscii(m);
	buf[3] = _getAscii(n);
}

int getRCommandBuffer(char *buf, int *sz, int address, int num)
{
	if (buf == NULL || sz == NULL ||
			(address < 0 || address > 255) || num < 0)
		return -1;

	buf[0] = 0x02;
	buf[1] = '0';

	_getAddressAscii(address, &buf[2]);

	num = num *2;

	buf[6] = _getAscii(num/10);
	buf[7] = _getAscii(num%10);

	buf[8] = 0x03;


	int i, sum = 0;
	for (i = 1; i <= 8; i++)
		sum += buf[i];

	i = sum&0xFF;
	buf[9]  = _getAscii(i/16);
	buf[10] = _getAscii(i%16);

	*sz = 11;

//	for (i = 0; i < *sz; i++) {
//		printf("%02x ", buf[i]);
//	}

//	printf("\n");

	return 0;
}

int getWCommandBuffer(char *buf, int *sz, int address, int num, char *data)
{
	if (buf == NULL || sz == NULL ||  
			(address < 0 || address > 255) || num < 0)  
		return -1; 

	buf[0] = 0x02;
	buf[1] = '1';

	_getAddressAscii(address, &buf[2]);

	num = num*2;

	buf[6] = _getAscii(num/10);
	buf[7] = _getAscii(num%10);

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
	buf[8+num*2+1]  = _getAscii(i/16);
	buf[8+num*2+2]  = _getAscii(i%16);

	*sz = 8+num*2+3;

//	for (i = 0; i < *sz; i++) {
//		printf("%02x ", buf[i]);
//	}

//	printf("\n");

	return 0;
}

int serial_stats_nsend(void)
{
	return _S->n_send;
}

int serial_stats_nrecv(void)
{
	return _S->n_recv;
}

int serial_stats_nerr(void)
{
	return _S->n_err;	
}
