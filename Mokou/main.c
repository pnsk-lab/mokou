/* $Id$ */

#include <stdio.h>
#include <unistd.h>

#include "mk_service.h"

int main(int argc, char** argv){
	if(getuid() != 0){
		fprintf(stderr, "Run me as root.\n");
		return 1;
	}
	mk_service_scan();
}
