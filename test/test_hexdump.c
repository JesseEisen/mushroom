#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{

		char payload[] = "12345678";
		int sz = strlen(payload);

		char hexdump[8092];
		char *p = hexdump;
	
		memset(hexdump, 0, sizeof(hexdump));
		int i;
		for (i = 0; i < sz; i++) {
			snprintf(p, 5, "0x%02x", payload[i]);
			p += 4;
		}

		printf("\n");
		printf("send message:MUSHROOM|%d|%s\n", sz, hexdump);
		
		return 0;
}
