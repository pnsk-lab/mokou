/* $Id$ */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char** argv){
	if(argc < 2){
		fprintf(stderr, "Usage: %s action [target]\n", argv[0]);
		fprintf(stderr, "Actions:\n");
		fprintf(stderr, "\treload   Reloads the service direcetory\n");
		fprintf(stderr, "\tstart    Start the service\n");
		fprintf(stderr, "\tstop     Stop the service\n");
		return 1;
	}
	char* msg = NULL;
	if(strcmp(argv[1], "reload") == 0){
		msg = malloc(3);
		msg[0] = 'R';
		msg[1] = '\n';
		msg[2] = 0;
	}else if(strcmp(argv[1], "start") == 0){
		if(argv[2] == NULL){
			fprintf(stderr, "Target is needed for this action\n");
			return 1;
		}
		msg = malloc(3 + strlen(argv[2]));
		msg[0] = 'U';
		strcpy(msg + 1, argv[2]);
		msg[1 + strlen(argv[2])] = '\n';
		msg[2 + strlen(argv[2])] = 0;
	}else if(strcmp(argv[1], "stop") == 0){
		if(argv[2] == NULL){
			fprintf(stderr, "Target is needed for this action\n");
			return 1;
		}
		msg = malloc(3 + strlen(argv[2]));
		msg[0] = 'D';
		strcpy(msg + 1, argv[2]);
		msg[1 + strlen(argv[2])] = '\n';
		msg[2 + strlen(argv[2])] = 0;
	}else{
		fprintf(stderr, "Unknown action: %s\n", argv[1]);
		return 1;
	}
	if(msg != NULL){
		int sock = socket(AF_LOCAL, SOCK_STREAM, 0);
		if(sock == -1){
			fprintf(stderr, "Socket creation failure\n");
			return 1;
		}
		struct sockaddr_un sun;
		sun.sun_family = AF_LOCAL;
		strcpy(sun.sun_path, "/tmp/mokou.sock");
		if(connect(sock, (struct sockaddr*)&sun, sizeof(sun)) == -1){
			fprintf(stderr, "Connection failure: %s\n", strerror(errno));
			close(sock);
		}
		send(sock, msg, strlen(msg), 0);
		
		char cbuf[2];
		cbuf[1] = 0;

		bool first = true;
		bool err = false;
		while(1){
			if(recv(sock, cbuf, 1, 0) <= 0) break;
			if(cbuf[0] == '\n'){
				break;
			}else if(cbuf[0] != '\r'){
				if(first && cbuf[0] == 'E'){
					err = true;
					fprintf(stderr, "Error: ");
				}else if(!first){
					fprintf(err ? stderr : stdout, "%c", cbuf[0]);
				}
				first = false;
			}
		}
		fprintf(err ? stderr : stdout, "\n");

		close(sock);
		free(msg);
	}
}
