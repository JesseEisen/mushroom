#ifndef _RDATA_H_
#define _RDATA_H_

void rdata_init(char *dbfile);
int rdata_test();

int rdata_set_controller(int id, int  status);
int rdata_get_controller(int id, int *status);

#endif
