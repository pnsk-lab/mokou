/* $Id$ */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mk_service.h"
#include "mk_server.h"
#include "mk_log.h"
#include "mk_util.h"
#include "mk_version.h"

extern bool mk_syslog;

int main(int argc, char** argv){
	int i;
	bool daemon = true;
	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			if(strcmp(argv[i], "--stderr") == 0 || strcmp(argv[i], "-S") == 0){
				mk_syslog = false;
			}else if(strcmp(argv[i], "-D") == 0){
				daemon = false;
			}else{
				fprintf(stderr, "%s: %s: unknown flag\n", argv[0], argv[i]);
				return 1;
			}
		}
	}
	if(getuid() != 0){
		fprintf(stderr, "Run me as root.\n");
		return 1;
	}
	char* log = mk_strcat3("Mokou version ", mk_get_version(), " starting up");
	mk_log(log);
	free(log);
	mk_service_scan();
	mk_start_services();
	mk_log("Mokou is up, creating the server socket");
	if(mk_server_init() != 0){
		mk_log("Could not initialize the server");
		return 1;
	}
	unsigned long long pid = 0;
	if(daemon){
		mk_log("Spawning daemon");
		pid = fork();
	}
	if(pid == 0){
		if(daemon){
			mk_log("Hello from daemon");
		}
		mk_log("Entering server loop");
		mk_server_loop();
		return 0;
	}else if(daemon){
		FILE* f = fopen("/var/run/mokou.pid", "w");
		fprintf(f, "%llu", pid);
		fclose(f);
	}
}
