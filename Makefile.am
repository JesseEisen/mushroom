AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS = mushroom
INCLUDES = -I./iniparser/src \
	-I./sqlite3  \
	-I./protobuf-c \
	-I./proto \
	-I./

mushroom_SOURCES = priqueue/posix_mq_queue.c \
	priqueue/local_queue.c \
	iniparser/src/dictionary.c \
	iniparser/src/iniparser.c \
	log.c \
	iniconfig.c \
	sqlite3/sqlite3.c \
	room.c \
	main.c \
	socket_client.c \
	protobuf-c/protobuf-c.c \
	proto/message_header.pb-c.c \
	proto/service.pb-c.c \
	util.c \
	cb.c \
	serial.c \
	debug.c

LIBS = -lpthread \
	-ldl \
	-lrt

CFLAGS += -g
