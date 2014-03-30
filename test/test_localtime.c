#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
	time_t t;
	time(&t);
	struct tm *tm = localtime(&t);
	printf( "sec:%d\n"
		"min:%d\n"
		"hour:%d\n"
		"mday:%d\n"
		"mon:%d\n"
		"year:%d\n"
		"wday:%d\n"
		"yday:%d\n"
		"isdst:%d\n",
		tm->tm_sec,
		tm->tm_min,
		tm->tm_hour,
		tm->tm_mday,
		tm->tm_mon+1,
		tm->tm_year+1900,
		tm->tm_wday,
		tm->tm_yday,
		tm->tm_isdst
	);

	printf("%02d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	printf("\n");

	return 0;
}
