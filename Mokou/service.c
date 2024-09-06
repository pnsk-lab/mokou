/* $Id$ */

#include "mk_service.h"

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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
		mk_log("Cleaning up the list");
	}
	services = malloc(sizeof(*services));
	services[0] = NULL;

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

				FILE* f = fopen(path, "r");
				if(f != NULL){
					struct stat s;
					stat(path, &s);
					char* buffer = malloc(s.st_size + 1);
					buffer[s.st_size] = 0;
					fread(buffer, s.st_size, 1, f);
					int i;
					int incr = 0;

					char* desc = NULL;
					char* exec = NULL;
					char* pidfile = NULL;
					
					for(i = 0;; i++){
						if(buffer[i] == '\n' || buffer[i] == 0){
							char oldc = buffer[i];
							buffer[i] = 0;

							char* line = buffer + incr;

							if(strlen(line) > 0 && line[0] != '#'){
								int j;

								for(j = 0; line[j] != 0; j++){
									if(line[j] == '='){
										line[j] = 0;
										
										char* key = line;
										char* value = line + j + 1;
										if(strcmp(key, "description") == 0){
											if(desc != NULL) free(desc);
											desc = mk_strdup(value);
										}else if(strcmp(key, "exec") == 0){
											if(exec != NULL) free(exec);
											exec = mk_strdup(value);
										}else if(strcmp(key, "pidfile") == 0){
											if(pidfile != NULL) free(pidfile);
											pidfile = mk_strdup(value);
										}
	
										break;
									}
								}
							}

							incr = i + 1;
							if(oldc == 0) break;
						}
					}
					fclose(f);

					bool bad = false;
					if(exec == NULL){
						char* log = mk_strcat(desc == NULL ? path : desc, ": Missing exec");
						mk_log(log);
						free(log);
						bad = true;
					}
					if(pidfile == NULL){
						char* log = mk_strcat(desc == NULL ? path : desc, ": Missing pidfile");
						mk_log(log);
						free(log);
						bad = true;
					}

					if(!bad){
						char* log = mk_strcat3("Adding ", desc == NULL ? path : desc, " to the list");
						mk_log(log);
						free(log);
					}

					if(desc != NULL) free(desc);
					if(exec != NULL) free(exec);
					if(pidfile != NULL) free(pidfile);
				}

				free(path);
			}
		}
		closedir(dir);
	}else{
		mk_log("Cannot open the directory.");
	}
}
