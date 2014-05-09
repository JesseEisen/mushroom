#ifndef _MRSH_H_
#define _MRSH_H_

// List of system moniter function APIs
char* dump_config(void);
int serial_stats_nsend(void);
int serial_stats_nrecv(void);
int serial_stats_nerr(void);

#endif
