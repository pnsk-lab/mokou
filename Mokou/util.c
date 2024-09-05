/* $Id$ */

#include "mk_util.h"

#include <stdlib.h>
#include <string.h>

char* mk_strcat(const char* a, const char* b){
	char* str = malloc(strlen(a) + strlen(b) + 1);
	memcpy(str, a, strlen(a));
	memcpy(str + strlen(a), b, strlen(b));
	str[strlen(a) + strlen(b)] = 0;
	return str;
}

char* mk_strcat3(const char* a, const char* b, const char* c){
	char* tmp = mk_strcat(a, b);
	char* str = mk_strcat(tmp, c);
	free(tmp);
	return str;
}

char* mk_strdup(const char* a){
	return mk_strcat(a, "");
}

bool mk_endswith(const char* str, const char* end){
	if(strlen(str) < strlen(end)) return false;
	int i;
	for(i = strlen(str) - strlen(end); str[i] != 0; i++){
		if(str[i] != end[i - strlen(str) + strlen(end)]) return false;
	}
	return true;
}
