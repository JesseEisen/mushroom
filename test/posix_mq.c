#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>

#define MSGQOBJ_NAME    "/myqueue1269"

struct Test {
	int x;
	int y;
	char hello[255];
};

void *thread_msg_send(void *arg)
{
	mqd_t msgq_id = (mqd_t)arg;
	printf("arg:%d\n", arg);

	int i = 0;
//	while (1) {
		struct Test *t = malloc(sizeof(struct Test));
		t->x = i++;
		t->y = i;
		sprintf(t->hello, "Hello World\n");

		char buf[4];
		int x = (int)t;
		buf[0] = (x >> 24)&0xFF;
		buf[1] = (x >> 16)&0xFF;
		buf[2] = (x >> 8)&0xFF;
		buf[3] = x&0xFF;

		printf("point address:%x, buf[0] = %x, buf[1] = %x, buf[2] = %x, buf[3] = %x\n", 
			x, buf[0], buf[1], buf[2], buf[3]);

		mq_send(msgq_id, buf, 4, 10);
//		sleep(1);
//	}
///
	return (void *)NULL;
}


void *thread_msg_recv(void *arg)
{
	mqd_t msgq_id = (mqd_t)arg;
	printf("arg:%d\n", arg);

	sleep(1);

//	while (1) {
		char buf[4];
		int ret = mq_receive(msgq_id, buf, 4, NULL);
		if (ret < 0) {
			perror("mq_receive error\n");
			exit(1);
		}

		int sz = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | (buf[3]&0xFF);

		struct Test *t = (struct Test *)sz;

		printf("point address:%x, buf[0] = %x, buf[1] = %x, buf[2] = %x, buf[3] = %x\n", 
			sz, buf[0], buf[1], buf[2], buf[3]);
		printf("x:%d, y:%d, %s\n", t->x, t->y, t->hello);
		free(t);
//	}

	return (void *)NULL;	
}
#define MAX_QMSG_SIZE       4

int main(void)
{
	mqd_t msgq_id;

	struct mq_attr attrs, actual_attrs;

	attrs.mq_flags = 0;
	attrs.mq_maxmsg = 100;
	attrs.mq_msgsize = MAX_QMSG_SIZE;
//	attrs.mq_curmsgs = 123;

//	msgq_id = mq_open(MSGQOBJ_NAME, O_RDWR);
	msgq_id = mq_open(MSGQOBJ_NAME, O_RDWR | O_CREAT /*| O_EXCL*/, S_IRWXU | S_IRWXG, &attrs);
	if (msgq_id < 0) {
		perror("mq_open error\n");
		exit(1);
	}


	mq_getattr(msgq_id, &actual_attrs);
	printf("Attributes right after queue creation:\n\t- non blocking flag: %d\n\t- maximum number of messages: %ld\n\t- maximum size of a message: %ld\n\t- current number of messages: %ld\n", (actual_attrs.mq_flags == 0 ? 0 : 1), actual_attrs.mq_maxmsg, actual_attrs.mq_msgsize, actual_attrs.mq_curmsgs);

	printf("%d\n", msgq_id);

	pthread_t tid_send, tid_recv;

	pthread_create(&tid_send, NULL, thread_msg_send, (void *)msgq_id);
	pthread_create(&tid_recv, NULL, thread_msg_recv, (void *)msgq_id);

	pthread_join(tid_send, NULL);
	pthread_join(tid_recv, NULL);

	mq_close(msgq_id);

	return 0;
}
