#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

void int24_encode(int x, unsigned char buf[4])
{
	buf[0] = (uint8_t)(x >> 16 & 0xff);
	buf[1] = (uint8_t)(x >> 8  & 0xff);
	buf[2] = (uint8_t)(x & 0xff);
//	buf[3] = (uint8_t)(x >> 24 & 0xff);
}

void int24_decode(unsigned char buf[3], uint32_t *x)
{
//	*x = (uint32_t)(buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24);
	*x = (uint32_t)(buf[0] << 16 | buf[1] << 8 | buf[2]);
}

int main(int argc, char *argv[])
{
	char buf[3];

	uint32_t i;

	uint32_t max = 1 << 16;
	printf("max:%d\n", max);
	for (i = 0; i < max; i++) {
		uint32_t y = i;
		int24_encode(y, buf);
		int24_decode(buf, &y);
		if (y != i) {	
			printf("error,i = %u, y = %u\n", i, y);
			return -1;
		}
	}
		
	return 0;
}
