// str.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_STR_H__
#define __HAVE_STR_H__

typedef struct cstr_struct cstr;
struct cstr_struct {
	int len;
	char* data;
};

typedef struct str_struct* str;
struct str_struct {
	int len;
	char data[0];
};

cstr Cstr(char* a, int l);

str Str(char* fmt, ...);
str new_str(char* fmt, va_list args); 

str char_str(char* a, int l);
str int_str(int a);
str obj_str(void* a);
str num_str(double a);

#define str_int(a) (a ? strtol(a->data,NULL,0) : 0)
#define str_num(a) (a ? strtod(a->data,NULL,0) : 0)
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

#endif
