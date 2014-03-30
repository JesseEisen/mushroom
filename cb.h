#ifndef _CB_H_
#define _CB_H_

#include "proto/message_header.pb-c.h"

struct socket_client;
typedef int (*MessageCallback)(struct socket_client *sc, void *ud, MessageHeader *header, void *msg, int sz);

typedef	void (*pb_init)(void *);
typedef	size_t (*pb_get_packed_size)(const void *);
typedef	size_t (*pb_pack)(const void *, uint8_t *out);
typedef	void* (*pb_unpack)(
				ProtobufCAllocator *,
				size_t len,
				const uint8_t *data);
typedef	void (*pb_free_unpacked)(void *, ProtobufCAllocator *);

struct protobuf_message {
	char name[255];
	pb_init            init;
	pb_get_packed_size get_packed_size;
	pb_pack            pack;
	pb_unpack          unpack;
	pb_free_unpacked   free_unpacked;
};

int protobuf_message_init(void);
int protobuf_message_register(char *name, pb_init init,
						pb_get_packed_size get_packed_size,
						pb_pack pack,
						pb_unpack unpack,
						pb_free_unpacked free_unpacked);
struct protobuf_message* protobuf_message_find(char *name);
void protobuf_message_dump(void);
void protobuf_message_destroy(void);

struct callback {
	int message_id;
	void *ud;
	MessageCallback cb;

#define FREE_BY_OTHER 0xab
#define FREE_BY_SELF 0xac
	int ud_option;
};


// 由于指针兼容性问题， 外部如果想要注册一个callback， 请使用宏定义， 避免直接使用callback_register
#define PROTOBUF_MESSAGE_REGISTER(NAME, X, Y, Z, M, N) \
	protobuf_message_register(NAME, (pb_init)X, (pb_get_packed_size)Y, (pb_pack)Z, (pb_unpack)M, (pb_free_unpacked)N)

int callback_register(int message_id, void *ud, int ud_option, MessageCallback cb);


void callback_init(void);
struct callback *callback_find(int message_id);
void callback_destroy(void);
void callback_dump(void);
int callback_set_userdata(int message_id, void *ud, int ud_option);
int callback_del_userdata(int message_id);

int pb_message_pack(void *pb, struct protobuf_message *opt, char **msg, int *sz);

// Helper function
struct room_model;
struct config;
int send_read_time_request(struct socket_client *sc);
int send_room_sensor_data(struct socket_client *sc, struct room_model *rm, int room_id);
int send_init_configure(struct socket_client *sc, struct room_model *rm, struct config *config);

#endif
