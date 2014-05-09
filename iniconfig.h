#ifndef _INI_CONFIG_H_
#define _INI_CONFIG_H_

struct iniconfig {
	int debug_info;
	int debug_warning;
	int debug_err;

	int log_info;
	int log_warning;
	int log_err;

	char log[255];
	char serial[255];
	char db[255];
	char endpoint[255];

	int freq_push_rdata;
	int freq_read_serial;
};

void iniconfig_init(char *inifile, struct iniconfig *ini);
int iniconfig_test();

#endif
