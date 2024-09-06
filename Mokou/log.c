/* $Id$ */

#include "mk_log.h"

#include <syslog.h>
#include <stdio.h>
#include <stdbool.h>

bool mk_syslog = true;

void mk_log(const char* log){
	if(mk_syslog){
		syslog(LOG_INFO, log);
	}else{
		fprintf(stderr, "%s\n", log);
	}
}
