all:
	rm -rf build/
	rm -rf a.out
	mkdir -p build
	make test_server
	make test_posix_mq
	make int24_codec
test_server:
	gcc ../debug.c test_server.c ../util.c ../proto/*.c ../protobuf-c/*.c -I../ -I../protobuf-c -o build/test_server

test_posix_mq:
	gcc posix_mq.c -lrt -o build/test_posix_mq
int24_codec:
	gcc int24_codec.c -o build/int24_codec

clean:
	rm -rf build/
	rm -rf a.out
