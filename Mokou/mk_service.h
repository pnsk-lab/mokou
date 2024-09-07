/* $Id$ */

#ifndef __MK_SERVICE_H__
#define __MK_SERVICE_H__

#include <stdbool.h>

struct mk_service {
	char* name;
	char* description;
	char* exec;
	char* pidfile;
	char* stop;
	bool stopped;
};

void mk_service_scan(void);
int mk_start_service(const char* name);
int mk_stop_service(const char* name);
void mk_start_services(void);
void mk_resurrect_services(void);

#endif
