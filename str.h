// str.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_STR_H__
#define __HAVE_STR_H__

#include <stdarg.h>
#include "defines.h"

typedef struct cstr_struct* cstr;
struct cstr_struct {
	int len;
	const char* data;
};

typedef struct str_struct* str;
struct str_struct {
	int len;
	char data[0];
};

cstr Cstr(const char* a, int l);

str Str(const char* fmt, ...);
str new_str(const char* fmt, va_list args); 
str copy(str s);

char* str_char(str);
str char_str(const char* a, int l);
str int_str(int a);
str obj_str(void* a);
str num_str(double a);

#define str_int(a) (a ? strtol(a->data,NULL,0) : 0)
#define str_num(a) (a ? strtod(a->data,NULL,0) : 0)
#define str_obj(a,t) (t)str_int(a)
#define type_str_func(n,t,f)\
str \
n (t a) { \
	char* tmp; \
	int len = asprintf(&tmp,f,a); \
	str retval = char_str(tmp,len); \
	free(tmp); \
	return retval; \
} \

#define cmp_str(a,b) \
	(a->len == b->len ? !strncmp(a->data,b->data,a->len) : 0)

#define icmp_str(a,b) \
	(a->len == b->len ? !strncasecmp(a->data,b->data,a->len) : 0)

#define ncmp_str(a,b,n) \
	(a->len >= n && b->len >= n ? ! strncmp(a->data,b->data,n) : 0)

int lesser_str(str a, str b);
str name_field(str line);
str skip_fields(str line, int n);
str dequote(str line);
str sub_str(str s, int start, int end);
str singlequote(str s);

#endif
