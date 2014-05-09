#include "mrsh.h"
#include "iniconfig.h"
#include "3rd/iniparser/iniparser.h"

#include <stdio.h>
#include <stdlib.h>

static int _MAGICNO = 0;
static char _dump[4096];
static struct iniconfig _C;
		
void iniconfig_init(char *inifile, struct iniconfig *config)
{
	int err = 0;

	dictionary *dict;
	dict = iniparser_load(inifile);
	if (dict == NULL) {
		fprintf(stderr, "[iniconfig] cannot parse file\n");
		exit(EXIT_FAILURE);
	}

	config->debug_info = iniparser_getint(dict, "mrshd:debug_info", -1);
	if (config->debug_info == -1) {
		fprintf(stderr, "[iniconfig] do not find debug_info\n");
		err = 1;
	}

	config->debug_warning = iniparser_getint(dict, "mrshd:debug_warning", -1);
	if (config->debug_warning == -1) {
		fprintf(stderr, "[iniconfig] do not find debug_warning\n");
		err = 1;
	}

	config->debug_err = iniparser_getint(dict, "mrshd:debug_err", -1);
	if (config->debug_err == -1) {
		fprintf(stderr, "[iniconfig] do not find debug_err\n");
		err = 1;
	}

	config->log_info = iniparser_getint(dict, "mrshd:log_info", -1);
	if (config->log_info == -1) {
		fprintf(stderr, "[iniconfig] do not find log_info\n");
		err = 1;
	}

	config->log_warning = iniparser_getint(dict, "mrshd:log_warning", -1);
	if (config->log_warning == -1) {
		fprintf(stderr, "[iniconfig] do not find log_warning\n");
		err = 1;
	}

	config->log_err = iniparser_getint(dict, "mrshd:log_err", -1);
	if (config->log_err == -1) {
		fprintf(stderr, "[iniconfig] do not find log_err\n");
		err = 1;
	}

	char *endpoint = iniparser_getstring(dict, "mrshd:endpoint", NULL);
	if (endpoint == NULL) {
		fprintf(stderr, "[iniconfig] do not find endpoint\n");
		err = 1;
	} else {
		sprintf(config->endpoint, "%s", endpoint);
	}

	char *serial = iniparser_getstring(dict, "mrshd:serial", NULL);
	if (serial == NULL) {
		fprintf(stderr, "[iniconfig] do not find serial\n");
		err = 1;
	} else {
		sprintf(config->serial, "%s", serial);
	}

	char *db = iniparser_getstring(dict, "mrshd:db", NULL);
	if (db == NULL) {
		fprintf(stderr, "[iniconfig] do not find db\n");
		err = 1;
	} else {
		sprintf(config->db, "%s", db);
	}

	char *log = iniparser_getstring(dict, "mrshd:log", NULL);
	if (db == NULL) {
		fprintf(stderr, "[iniconfig] do not find log\n");
		err = 1;
	} else {
		sprintf(config->log, "%s", log);
	}

	config->freq_read_serial = iniparser_getint(dict, "mrshd:freq_read_serial", -1);
	if (config->freq_read_serial == -1) {
		fprintf(stderr, "[iniconfig] do not find freq_read_serial\n");
		err = 1;
	}

	config->freq_push_rdata = iniparser_getint(dict, "mrshd:freq_push_rdata", -1);
	if (config->freq_push_rdata == -1) {
		fprintf(stderr, "[iniconfig] do not find freq_push_rdata\n");
		err = 1;
	}

	iniparser_freedict(dict);
	
	if (err) {
		exit(EXIT_FAILURE);
	}

	_C = *config;

	_MAGICNO = 0x1234;
}

int iniconfig_test()
{
	if (_MAGICNO != 0x1234) {
		fprintf(stderr, "iniconfig module do not init\n");
		return -1;
	}

	return 0;
}


char *dump_config(void)
{
	int offsize = 0;
	memset(_dump, 0, sizeof(_dump));
	offsize  = sprintf(_dump, "debug_info:%d\n", _C.debug_info);
	offsize += sprintf(_dump+offsize, "debug_warning:%d\n", _C.debug_warning);
	offsize += sprintf(_dump+offsize, "debug_err:%d\n", _C.debug_err);

	offsize += sprintf(_dump+offsize, "log_info:%d\n", _C.log_info);
	offsize += sprintf(_dump+offsize, "log_warning:%d\n", _C.log_warning);
	offsize += sprintf(_dump+offsize, "log_err:%d\n", _C.log_err);

	offsize += sprintf(_dump+offsize, "log:%s\n", _C.log);
	offsize += sprintf(_dump+offsize, "serial:%s\n", _C.serial);
	offsize += sprintf(_dump+offsize, "db:%s\n", _C.db);
	offsize += sprintf(_dump+offsize, "endpoint:%s\n", _C.endpoint);

	offsize += sprintf(_dump+offsize, "freq_push_rdata:%d\n", _C.freq_push_rdata);
	offsize += sprintf(_dump+offsize, "freq_read_serial:%d\n", _C.freq_read_serial);

	return _dump;
}
