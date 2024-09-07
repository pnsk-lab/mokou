/* $Id$ */

#include "mk_server.h"

#include "mk_service.h"
#include "mk_version.h"
#include "mk_util.h"
#include "mk_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

struct sockaddr_un sun;
int server;
extern const char* mk_errors[];

int mk_server_init(void){
	remove("/tmp/mokou.sock");
	memset(&sun, 0, sizeof(sun));
	server = socket(AF_LOCAL, SOCK_STREAM, 0);
	if(server == -1){
		mk_log("Socket creation failure");
		return 1;
	}
	sun.sun_family = AF_LOCAL;
	strcpy(sun.sun_path, "/tmp/mokou.sock");
	if(bind(server, (struct sockaddr*)&sun, sizeof(sun)) == -1){
		mk_log("Bind failure");
		close(server);
		return 1;
	}
	if(listen(server, 16) == -1){
		mk_log("Listen failure");
		close(server);
		return 1;
	}
	return 0;
}

#define PROTOCOL_ERROR "EProtocol Error\n"

void mk_server_loop(void){
	struct sockaddr_un cun;
	socklen_t socklen = sizeof(cun);
	char* ver = mk_strcat3("R", mk_get_version(), "\n");
	char cbuf[2];
	cbuf[1] = 0;
	char* str = malloc(1);
	str[0] = 0;
	struct pollfd pollfds[16 + 1];
	pollfds[0].fd = server;
	pollfds[0].events = POLLIN | POLLPRI;
	while(1){
		mk_log("Waiting for the connection");
		int r = poll(pollfds, 16 + 1, 5000);
		int cli = accept(server, (struct sockaddr*)&cun, &socklen);
		send(cli, ver, strlen(ver), 0);
		while(1){
			if(recv(cli, cbuf, 1, 0) <= 0) break;
			if(cbuf[0] == '\n'){
				if(str[0] == 'U'){
					int err = mk_start_service(str + 1);
					if(err != 0){
						send(cli, "E", 1, 0);
						send(cli, mk_errors[err], strlen(mk_errors[err]), 0);
						send(cli, "\n", 1, 0);
					}else{
						send(cli, "Mok\n", 4, 0);
					}
				}else if(str[0] == 'D'){
					int err = mk_stop_service(str + 1);
					if(err != 0){
						send(cli, "E", 1, 0);
						send(cli, mk_errors[err], strlen(mk_errors[err]), 0);
						send(cli, "\n", 1, 0);
					}else{
						send(cli, "Mok\n", 4, 0);
					}
				}else{
					send(cli, PROTOCOL_ERROR, strlen(PROTOCOL_ERROR), 0);
				}
				free(str);
				str = malloc(1);
				str[0] = 0;
				break;
			}else if(cbuf[0] != '\r'){
				char* tmp = str;
				str = mk_strcat(tmp, cbuf);
				free(tmp);
			}
		}
		close(cli);
	}
	free(ver);
	free(ver);
}
