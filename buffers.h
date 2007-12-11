// buffers.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __BUFFERS_H__
#define __BUFFERS_H__

#include <sys/types.h>
#include "defines.h"
#include "str.h"


typedef struct buffer_struct* Buffer;
struct buffer_struct {
	Buffer next;
	size_t length;
	size_t pos;
	char data[Max_Buffer_Size];
};

Buffer new_buffer(Buffer buf, int pos);
Buffer seek_buffer(Buffer buf, int pos);
int find_buffer(Buffer buf, int pos, char* delim);
char fetch_buffer(Buffer buf, int pos);
Buffer read_buffer(Buffer dst, Buffer src, int pos, int len);
Buffer write_buffer(Buffer dst, char* src, int len);
Buffer reverse_buffer(Buffer buf, Buffer last);
int search_buffer(Buffer buf, int pos, str key, int off);
Buffer print_buffer(Buffer buf, char* fmt, ...);
Buffer write_str(Buffer dst, str src);
str read_str(Buffer src, int pos, int len);
void dump_buffer(Buffer src, int pos);
str readline_buffer(Buffer buf, int pos);
int skipheaders_buffer(Buffer src, int pos);
Buffer dechunk_buffer(Buffer src);
int length_buffer(Buffer buf);

#endif
