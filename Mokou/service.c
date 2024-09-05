/* $Id$ */

#include "mk_service.h"

#include <dirent.h>
#include <stdlib.h>

#include "mk_log.h"
#include "mk_util.h"

struct mk_service** services = NULL;

void mk_service_scan(void){
	if(services != NULL){
		int i;
		for(i = 0; services[i] != NULL; i++){
			free(services[i]->name);
			free(services[i]->exec);
			free(services[i]->pidfile);
			free(services[i]);
		}
		free(services);
	}

	mk_log("Scanning the service directory.");

	DIR* dir = opendir(PREFIX "/etc/mokou");
	if(dir != NULL){
		struct dirent* d;
		while((d = readdir(dir)) != NULL){
			if(mk_endswith(d->d_name, ".conf")){
				char* path = mk_strcat(PREFIX "/etc/mokou/", d->d_name);
				char* str = mk_strcat("Reading ", path);
				mk_log(str);
				free(str);
				free(path);
			}
		}
		closedir(dir);
	}else{
		mk_log("Cannot open the directory.");
	}
}
