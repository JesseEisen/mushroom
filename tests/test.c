#include <stdio.h>

struct Test {
	char hello[24];
	int x;
};

int main(int argc, char *argv[])
{
	struct Test t1 = {
		.hello = "Hello World",
		.x = 20
	};

	struct Test t2 = t1;

	printf("%s, %d\n", t2.hello, t2.x);

	int offsize = 0;
	char buf[128];
	offsize = sprintf(buf, "%s", "Hello World");
	offsize = sprintf(buf+offsize, "H");
	
	printf("offsize = %d, buf = %s\n", offsize, buf);
		
	return 0;
}
