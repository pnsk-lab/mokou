/* $Id$ */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "mk_service.h"
#include "mk_log.h"
#include "mk_util.h"
#include "mk_version.h"

int main(int argc, char** argv){
	if(getuid() != 0){
		fprintf(stderr, "Run me as root.\n");
		return 1;
	}
	char* log = mk_strcat3("Mokou version ", mk_get_version(), " starting up");
	mk_log(log);
	free(log);
	mk_service_scan();
}
