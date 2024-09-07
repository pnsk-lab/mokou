/* $Id$ */

#include "mk_service.h"

#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "mk_log.h"
#include "mk_util.h"

struct mk_service** services = NULL;

#ifdef __linux__
const char* sys_signame[] = {
	"",
	"HUP",
	"INT",
	"QUIT",
	"ILL",
	"TRAP",
	"ABRT",
	"BUS",
	"FPE",
	"KILL",
	"USR1",
	"SEGV",
	"USR2",
	"PIPE",
	"ALRM",
	"TERM",
	"STKFLT",
	"CHLD",
	"CONT",
	"STOP",
	"TSTP",
	"TTIN",
	"TTOU",
	"URG",
	"XCPU",
	"XFSZ",
	"VTALRM",
	"PROF",
	"WINCH",
	"POLL",
	"PWR",
	"SYS",
	"RTMIN"
};
#endif

void mk_service_scan(void){
	if(services != NULL){
		int i;
		for(i = 0; services[i] != NULL; i++){
			if(services[i]->name != NULL) free(services[i]->name);
			if(services[i]->stop != NULL) free(services[i]->stop);
			if(services[i]->description != NULL) free(services[i]->description);
			if(services[i]->exec != NULL) free(services[i]->exec);
			if(services[i]->pidfile != NULL) free(services[i]->pidfile);
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
					char* stop = NULL;
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
										}else if(strcmp(key, "stop") == 0){
											if(stop != NULL) free(stop);
											stop = mk_strdup(value);
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

						int i;
						struct mk_service* serv = malloc(sizeof(*serv));
						serv->name = mk_strdup(d->d_name);

						for(i = strlen(d->d_name) - 1; i >= 0; i--){
							if(serv->name[i] == '.'){
								serv->name[i] = 0;
								break;
							}
						}

						serv->description = desc != NULL ? mk_strdup(desc) : NULL;
						serv->stop = stop != NULL ? mk_strdup(stop) : NULL;
						serv->exec = mk_strdup(exec);
						serv->pidfile = mk_strdup(pidfile);
						serv->stopped = false;

						struct mk_service** oldsrvs = services;
						for(i = 0; oldsrvs[i] != NULL; i++);
						services = malloc(sizeof(*services) * (i + 2));
						for(i = 0; oldsrvs[i] != NULL; i++){
							services[i] = oldsrvs[i];
						}
						services[i] = serv;
						services[i + 1] = NULL;
						free(oldsrvs);
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

const char* mk_errors[] = {
	"Success",
	"No such service",
	"Service is alive",
	"Failed to start",
	"Service is dead",
	"Bad signal",
	"Could not stop the service",
	"Could not run the stop command"
};

int mk_stop_service(const char* name){
	int i;
	for(i = 0; services[i] != NULL; i++){
		if(strcmp(services[i]->name, name) == 0){
			struct mk_service* srv = services[i];
			char* log = mk_strcat("Stopping ", name);
			mk_log(log);
			free(log);

			bool alive = false;

			FILE* f = fopen(srv->pidfile, "r");
			unsigned long long pid;
			if(f != NULL){
				fscanf(f, "%llu", &pid);
				fclose(f);
				alive = kill(pid, 0) == 0;
			}

			if(!alive){
				mk_log("Process seems to be dead, not stopping");
				return 4;
			}

			if(srv->stop == NULL || srv->stop[0] == '#'){
				int sig = -1;
				if(srv->stop == NULL){
					sig = SIGINT;
				}
				if(sig == -1){
					int i;
					for(i = 1; i < NSIG; i++){
						if(strcmp(sys_signame[i], srv->stop + 1) == 0){
							sig = i;
							break;
						}
					}
				}
				if(sig == -1){
					mk_log("Bad signal");
					return 5;
				}else{
					log = mk_strcat("Sending SIG", sys_signame[sig]);
					mk_log(log);
					free(log);
					kill(pid, sig);
				}
			}else{
				char** pargv = malloc(sizeof(*pargv));
				pargv[0] = NULL;
	
				int i;
				int incr = 0;
				for(i = 0;; i++){
					if(srv->stop[i] == 0 || srv->stop[i] == ' '){
						char* str = malloc(i - incr + 1);
						memcpy(str, srv->stop + incr, i - incr);
						str[i - incr] = 0;
	
						char** oldargv = pargv;
						int j;
						for(j = 0; oldargv[j] != NULL; j++);
						pargv = malloc(sizeof(*pargv) * (j + 2));
						for(j = 0; oldargv[j] != NULL; j++) pargv[j] = oldargv[j];
						pargv[j] = str;
						pargv[j + 1]  = NULL;
						free(oldargv);
	
						incr = i + 1;
						if(srv->exec[i] == 0) break;
					}
				}

				bool fail = false;
				pid_t pid = fork();
				if(pid == 0){
					int n = open("/dev/null", O_RDWR);
					dup2(n, 1);
					dup2(n, 2);
					execvp(pargv[0], pargv);
					_exit(-1);
				}else{
					int status;
					waitpid(pid, &status, 0);
					if(WEXITSTATUS(status) != 0) fail = true;
				}

				for(i = 0; pargv[i] != NULL; i++) free(pargv[i]);
				free(pargv);

				if(fail){
					mk_log("Failed to run stop command");
					return 7;
				}
			}
			
			usleep(100);

			bool dead = false;
			for(i = 0; i < 3; i++){
				if(kill(pid, 0) == -1){
					mk_log("Process died");
					dead = true;
					break;
				}else{
					mk_log("Process is still alive");
				}
				if(i != 2) sleep(1);
			}
			if(!dead){
				mk_log("Could not kill the process");
				return 6;
			}

			srv->stopped = true;
			return 0;
		}
	}
	return 1;
}

int mk_start_service(const char* name){
	int i;
	for(i = 0; services[i] != NULL; i++){
		if(strcmp(services[i]->name, name) == 0){
			struct mk_service* srv = services[i];
			char* log = mk_strcat("Starting ", name);
			mk_log(log);
			free(log);

			bool alive = false;

			FILE* f = fopen(srv->pidfile, "r");
			if(f != NULL){
				unsigned long long pid;
				fscanf(f, "%llu", &pid);
				fclose(f);
				alive = kill(pid, 0) == 0;
			}
			if(alive){
				mk_log("Process seems to be alive, not starting");
				return 2;
			}

			char** pargv = malloc(sizeof(*pargv));
			pargv[0] = NULL;

			int i;
			int incr = 0;
			for(i = 0;; i++){
				if(srv->exec[i] == 0 || srv->exec[i] == ' '){
					char* str = malloc(i - incr + 1);
					memcpy(str, srv->exec + incr, i - incr);
					str[i - incr] = 0;

					char** oldargv = pargv;
					int j;
					for(j = 0; oldargv[j] != NULL; j++);
					pargv = malloc(sizeof(*pargv) * (j + 2));
					for(j = 0; oldargv[j] != NULL; j++) pargv[j] = oldargv[j];
					pargv[j] = str;
					pargv[j + 1]  = NULL;
					free(oldargv);

					incr = i + 1;
					if(srv->exec[i] == 0) break;
				}
			}

			bool fail = false;

			pid_t pid = fork();
			if(pid == 0){
				int n = open("/dev/null", O_RDWR);
				dup2(n, 1);
				dup2(n, 2);
				execvp(pargv[0], pargv);
				_exit(-1);
			}else{
				int status;
				waitpid(pid, &status, 0);
				if(WEXITSTATUS(status) != 0) fail = true;
			}
			for(i = 0; pargv[i] != NULL; i++) free(pargv[i]);
			free(pargv);
			if(fail){
				log = mk_strcat("Failed to start ", name);
				mk_log(log);
				free(log);
				srv->stopped = false;
				return 3;
			}else{
				log = mk_strcat("Started ", name);
				mk_log(log);
				free(log);
				srv->stopped = false;
			}

			return 0;
		}
	}
	return 1;
}

void mk_start_services(void){
	int i;
	for(i = 0; services[i] != NULL; i++){
		mk_start_service(services[i]->name);
	}
}

void mk_resurrect_services(void){
	int i;
	bool re = false;
	for(i = 0; services[i] != NULL; i++){
		if(!services[i]->stopped){
			bool alive = false;

			FILE* f = fopen(services[i]->pidfile, "r");
			if(f != NULL){
				unsigned long long pid;
				fscanf(f, "%llu", &pid);
				fclose(f);
				alive = kill(pid, 0) == 0;
			}
			if(!alive){
				if(!re){
					mk_log("Resurrection");
					re = true;
				}
				mk_start_service(services[i]->name);
			}
		}
	}
}
