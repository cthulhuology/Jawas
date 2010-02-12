// str.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_STR_H__
#define __HAVE_STR_H__

#include <stdarg.h>
#include "defines.h"
#include "memory.h"

typedef int (*test_t)(char x, char y);

typedef struct str_struct* str;
struct str_struct {
	int length;
	char* data;
	char contents[0];
};

int min(int a, int b);
int max(int a, int b);
int between(int a, int x, int b);

str set(str s, int pos, char c);
char at(str s, int pos);
int len(str s);
str ref(const char* a, int l);
str copy(const char* a, int l);
str blank(int l);
str clone(str s);
str from(str s, int start, int len);
str append(str s, str v);

str int_str(int i);
str hex_str(int i);
str obj_str(void* p);

int str_int(str a);
int str_hex(str a);
#define str_obj(a,t) (t)str_hex(a)

str $(const char* fmt, ...);
str new_str(const char* fmt, va_list args); 

str name_field(str line);
str skip_fields(str line, int n);
str dequote(str line);

int cmp(str a, str b);
int ncmp(str a, str b,int n);
int icmp(str a, str b);
int lesser(str a, str b);

int find(str src, int pos, char* delim, size_t dl);
int search(str src, int pos, str key);
str read_line(str src, int pos);
int skip_headers(str src, int pos);
str dechunk(str src);

#endif
