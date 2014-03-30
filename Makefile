#CC = arm-none-linux-gnueabi-gcc
CC = gcc

SRCS += priqueue/posix_mq_queue.c \
	priqueue/local_queue.c \
	iniparser/src/*.c \
	log.c \
	config.c \
	sqlite3/sqlite3.c \
	room.c \
	main.c \
	socket_client.c \
	protobuf-c/protobuf-c.c \
	proto/*.c \
	util.c \
	cb.c \
	serial.c \
	debug.c
	

CFLAGS += -lpthread \
	-ldl \
	-I./iniparser/src \
	-I./sqlite3  \
	-I./protobuf-c \
	-I./proto \
	-g \
	-I./  \
	-lrt
	

mr:
	$(CC) $(SRCS) $(CFLAGS) -o mushroom

clean:
	rm -rf mr

valgrind:
	valgrind --leak-check=full -v  --show-reachable=yes --track-origins=yes ./mushroom
