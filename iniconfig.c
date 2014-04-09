#include "base.h"
#include "iniconfig.h"
#include "iniparser/src/iniparser.h"

int config_load(char *ini_file, struct config *config)
{
	assert(config);
	assert(ini_file);

	memset(config, 0, sizeof(struct config));


	dictionary *ini;
	ini = iniparser_load(ini_file);
	if (ini == NULL) {
		DEBUG("cannot parse config file\n");
		return -1;
	}	

	config->daemon = iniparser_getint(ini, "sys:daemon", DEAMON);
	config->debug_info = iniparser_getint(ini, "sys:debug_info", DEBUG_INFO);
	config->debug_warning = iniparser_getint(ini, "sys:debug_warning", DEBUG_WARNING);
	config->debug_trace = iniparser_getint(ini, "sys:debug_trace", DEBUG_TRACE);
	config->debug_err = iniparser_getint(ini, "sys:debug_err", DEBUG_ERR);

	char *remote_ip = iniparser_getstring(ini, "sys:remote_ip", NULL);	
	assert(remote_ip);
	sprintf(config->remote_ip, "%s", remote_ip);


	config->remote_port = iniparser_getint(ini, "sys:remote_port", -1);

	sprintf(config->serial_path, "%s", iniparser_getstring(ini, "sys:serial", SERIAL_PATH));
	sprintf(config->db_path, "%s", iniparser_getstring(ini, "sys:db", DB_PATH));
	sprintf(config->log_info_path, "%s", iniparser_getstring(ini, "sys:log_info", LOG_INFO_PATH));
	sprintf(config->log_warning_path, "%s", iniparser_getstring(ini, "sys:log_warning", LOG_WARNING_PATH));
	sprintf(config->log_trace_path, "%s", iniparser_getstring(ini, "sys:log_trace", LOG_TRACE_PATH));
	sprintf(config->log_err_path, "%s", iniparser_getstring(ini, "sys:log_err", LOG_ERR_PATH));

	config->heartbeat = iniparser_getint(ini, "param:heartbeat", HEARTBEAT);
	config->time_sync_freq = iniparser_getint(ini, "param:time_sync_freq", TIME_SYNC_FREQ);
	config->read_plc_freq = iniparser_getint(ini, "param:read_plc_freq", READ_PLC_FREQ);
	config->sensor_data_push_freq = iniparser_getint(ini, "param:sensor_data_push_freq", SENSOR_DATA_PUSH_FREQ);	

	iniparser_freedict(ini);

	config->BASE.magic = MAGIC_NUMBER;

	return 0;
}

int config_dump(struct config *config)
{
	assert(config);
	if (config->BASE.magic != MAGIC_NUMBER) {
		DEBUG("config module do not init.\n");
		return -1;
	}

	DEBUG("daemon:               %d\n", config->daemon);
	DEBUG("debug_info:           %d\n", config->debug_info);
	DEBUG("debug_warning:        %d\n", config->debug_warning);
	DEBUG("debug_trace:          %d\n", config->debug_trace);
	DEBUG("debug_err:            %d\n", config->debug_err);
	DEBUG("remote_ip:            %s\n", config->remote_ip);
	DEBUG("remote_port:          %d\n", config->remote_port);
	DEBUG("serial_path:          %s\n", config->serial_path);
	DEBUG("db_path:              %s\n", config->db_path);
	DEBUG("log_info_path:        %s\n", config->log_info_path);
	DEBUG("log_warning_path:     %s\n", config->log_warning_path);
	DEBUG("log_trace_path:       %s\n", config->log_trace_path);
	DEBUG("log_err_path:         %s\n", config->log_err_path);
	DEBUG("heartbeat:            %d\n", config->heartbeat);
	DEBUG("time_sync_freq:       %d\n", config->time_sync_freq);
	DEBUG("read_plc_freq:        %d\n", config->read_plc_freq);
	DEBUG("sensor_data_push_freq:%d\n", config->sensor_data_push_freq);

	return 0;
}
