// buffers.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pages.h"
#include "buffers.h"

Buffer
new_buffer(Buffer buf, int pos)
{
	Buffer retval = (Buffer)new_page();
	memset(retval->data,0,Max_Buffer_Size);
	retval->length = 0;
	retval->next = buf;
	retval->pos = pos;
	return retval;
}

Buffer
free_buffer(Buffer buf)
{
	if (! buf) return NULL;
	Buffer retval = buf->next;
	free_page((Page)buf);
	return retval;
}

Buffer
seek_buffer(Buffer buf, int pos)
{
	if (! buf) return NULL;
	if (pos < buf->pos) return seek_buffer(buf->next,pos);
	return buf;
}

Buffer
reverse_buffer(Buffer buf, Buffer last)
{
	Buffer tmp;
	tmp = buf->next;
	buf->next = last;
	if (tmp) return reverse_buffer(tmp,buf);
	return buf;
}

Buffer
write_buffer(Buffer dst, char* src, int len)
{
	if (! dst) dst = new_buffer(dst,0);
	int delta = Max_Buffer_Size - dst->length;
	if (delta >= len) {
		memcpy(dst->data+dst->length,src,len);	
		dst->length = dst->length + len;
		return dst;
	}
	memcpy(dst->data+dst->length,src,delta);
	dst->length = dst->length + delta;
	return write_buffer(new_buffer(dst,dst->length+dst->pos),src+delta,len-delta);
}

char
fetch_buffer(Buffer buf, int pos)
{
	Buffer tmp = seek_buffer(buf,pos);
	int delta = pos - tmp->pos;
	return tmp->data[delta];
}

Buffer
read_buffer(Buffer dst, Buffer src, int pos, int len)
{
	int delta;
	Buffer tmp;
	Buffer retval = new_buffer(dst,(dst ? dst->pos+dst->length : 0));
	tmp = seek_buffer(src,pos);
	if (!tmp) return retval;
	delta = pos - tmp->pos;
	if (delta + len <= tmp->length) {
		memcpy(retval->data,&tmp->data[delta],len);
		retval->length = len;
		return retval;
	}
	memcpy(retval->data,&tmp->data[delta],tmp->length - delta);
	retval->length = tmp->length - delta;
	return read_buffer(retval,src, pos + retval->length,len - retval->length);
}

char*
print_buffer(Buffer buf)
{
	int i;
	char* retval = (char*)malloc(length_buffer(buf)+1);
	for (i = 0; buf; buf = buf->next) {
		memcpy(retval+i,buf->data,buf->length);
		i += buf->length;
	}
	retval[i] = '\0';
	return retval;
}

int
length_buffer(Buffer buf)
{
	int total;
	if (! buf) return 0;
	for (total = 0; buf; buf = buf->next) total += buf->length;
	return total;
}

int
find_buffer(Buffer buf, int pos, char* delim, int len)
{
	int i,j,delta;
	Buffer tmp = seek_buffer(buf,pos);
	while (tmp) {	
		 delta = pos - buf->pos;
		for (i = delta; i < buf->length; ++i)
			for (j = 0; j < len; ++j ) 
				if (delim[j] == buf->data[i]) 
					return i + buf->pos;
		tmp = seek_buffer(buf,pos+buf->length - delta);
	}
	return -1;
}

