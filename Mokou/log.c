/* $Id$ */

#include "mk_log.h"

#include <syslog.h>

void mk_log(const char* log){
	syslog(LOG_INFO, log);
}
