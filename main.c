#include "iniconfig.h"
#include "log.h"
#include "serial.h"
#include "console.h"
#include "conn.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: ./mrshd config\n");
		exit(EXIT_FAILURE);
	}

	// 1. config module init
	struct iniconfig config;	
	iniconfig_init(argv[1], &config);
	if (iniconfig_test() != 0) {
		fprintf(stderr, "iniconfig module test failure.\n");
		exit(EXIT_FAILURE);
	}

	// 2. log module init
	log_init(config.log, config.debug_info, 
				config.debug_warning, 
				config.debug_err,
				config.log_info, 
				config.log_warning, 
				config.log_err);
	if (log_test() != 0) {
		fprintf(stderr, "log module test failure.\n");
		exit(EXIT_FAILURE);
	}	

	// 3. serial module init
//	serial_init(config.serial);
//	if (serial_test() != 0) {
//		fprintf(stderr, "serial module test failure.\n");
//		exit(EXIT_FAILURE);
//	}	

	// console module init
	console_init("127.0.0.1:5200");
	if (console_test() != 0) {
		fprintf(stderr, "console module test failure.\n");
		exit(EXIT_FAILURE);
	}

	conn_init(config.endpoint);
	if (conn_test() != 0) {
		fprintf(stderr, "conn module test failure.\n");
		exit(EXIT_FAILURE);
	}

	while (1) getchar();	
	return 0;
}
