/* $ID$ */

#ifndef __MK_UTIL_H__
#define __MK_UTIL_H__

#include <stdbool.h>

char* mk_strcat(const char* a, const char* b);
char* mk_strcat3(const char* a, const char* b, const char* c);
char* mk_strdup(const char* a);
bool mk_endswith(const char* str, const char* end);

#endif
