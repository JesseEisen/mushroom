#ifndef _INI_CONFIG_H_
#define _INI_CONFIG_H_

#define DEAMON 0
#define DEBUG_INFO 1
#define DEBUG_WARNING 1
#define DEBUG_TRACE 1
#define DEBUG_ERR 1

#define SERIAL_PATH "/dev/ttyUSB0"
#define DB_PATH "./db/mushroom.db"
#define LOG_INFO_PATH "./log/info.log"
#define LOG_WARNING_PATH "./log/warning.log"
#define LOG_TRACE_PATH "./log/trace.log"
#define LOG_ERR_PATH "./log/err.log"

#define HEARTBEAT 10
#define TIME_SYNC_FREQ 10
#define READ_PLC_FREQ 10
#define SENSOR_DATA_PUSH_FREQ 10

struct BASE;

struct config {
	struct BASE BASE;
	int daemon;
	int debug_info;
	int debug_warning;
	int debug_trace;
	int debug_err;

	char remote_ip[255];
	int remote_port;

	char serial_path[255];
	char db_path[255];
	char log_info_path[255];
	char log_warning_path[255];
	char log_trace_path[255];
	char log_err_path[255];

	int heartbeat;
	int time_sync_freq;
	int read_plc_freq;
	int sensor_data_push_freq;	
};

int config_load(char *ini_file, struct config *config);
int config_dump(struct config *config);

#endif
