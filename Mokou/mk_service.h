/* $Id$ */

#ifndef __MK_SERVICE_H__
#define __MK_SERVICE_H__

struct mk_service {
	char* description;
	char* exec;
	char* pidfile;
};

void mk_service_scan(void);

#endif
