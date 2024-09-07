/* $Id$ */

#include "mk_server.h"

#include "mk_version.h"
#include "mk_util.h"
#include "mk_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

struct sockaddr_un sun;
int server;

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

void mk_server_loop(void){
	struct sockaddr_un cun;
	socklen_t socklen = sizeof(cun);
	char* ver = mk_strcat3("V", mk_get_version(), "\n");
	while(1){
		mk_log("Waiting for the connection");
		int cli = accept(server, (struct sockaddr*)&cun, &socklen);
		send(cli, ver, strlen(ver), 0);
		close(cli);
	}
	free(ver);
}
