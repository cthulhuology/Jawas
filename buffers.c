// buffers.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "pages.h"
#include "alloc.h"
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
	int delta = 0;
	Buffer tmp = NULL;
	Buffer retval = new_buffer(dst,(dst ? dst->pos+dst->length : 0));
	tmp = seek_buffer(src,pos);
	if (!tmp) return retval;
	delta = pos - tmp->pos;
	debug("Read Buffer Delta is %i (pos %i) (tmp->pos %i)",delta,pos, tmp->pos);
	if (delta + len <= tmp->length) {
		memcpy(retval->data,&tmp->data[delta],len);
		retval->length = len;
		return retval;
	}
	memcpy(retval->data,&tmp->data[delta],tmp->length - delta);
	retval->length = tmp->length - delta;
	return read_buffer(retval,src, pos + retval->length,len - retval->length);
}

int
length_buffer(Buffer buf)
{
	int total = 0;
	if (! buf) return 0;
	for (total = 0; buf; buf = buf->next) total += buf->length;
	return total;
}

int
find_buffer(Buffer buf, int pos, char* delim)
{
	int i,j,delta, len = length_buffer(buf);
	Buffer tmp = seek_buffer(buf,pos);
	while (tmp) {
		delta = pos - tmp->pos;
		for (i = delta; i < tmp->length; ++i)
			for (j = 0; delim[j]; ++j ) 
				if (delim[j] == tmp->data[i]) 
					return i + tmp->pos;
		if (i + tmp->pos >= len) return len;
		pos = tmp->pos + tmp->length;
		tmp = seek_buffer(buf,pos);
	}
	return len;
}

int
search_buffer(Buffer buf, int pos, str key, int off)
{
	Buffer tmp = seek_buffer(buf,pos);
	int len = length_buffer(buf);
	int delta;
seek_again:
	if (pos >= tmp->pos + tmp->length)
		tmp = seek_buffer(buf,pos);
	delta = pos - tmp->pos;
	if (off >= key->len) return pos - key->len;
	if (pos >= len) return len;
	if (key->data[off] == tmp->data[delta]) {
		++pos;
		++off;	
		goto seek_again;
	}
	++pos;
	off = 0;
	goto seek_again;
}

Buffer
print_buffer(Buffer buf, char* fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	str s = new_str(fmt,args);
	return write_buffer(buf,s->data,s->len);
}

Buffer
write_str(Buffer dst, str src)
{
	return write_buffer(dst,src->data,src->len);
}

str
read_str(Buffer src, int pos, int len)
{
	int o = 0;
	int delta;
	Buffer tmp = NULL;
	str retval = (str)salloc(sizeof(struct str_struct) + len);
	retval->len = len;
	for ( tmp = seek_buffer(src,pos); tmp; tmp = seek_buffer(src,pos)) {
		delta = pos - tmp->pos;
		if (delta + (len-o) <= tmp->length) {
			memcpy(&retval->data[o],&tmp->data[delta],len-o);
			return retval;
		}
		memcpy(&retval->data[o],&tmp->data[delta],tmp->length - delta);
		o += tmp->length - delta;
		pos += tmp->length - delta;
	}
	return retval;
}

void
dump_buffer(Buffer src, int pos)
{
	Buffer tmp;
	for (tmp = seek_buffer(src,pos); tmp; tmp = tmp->next) {
		write(2,tmp->data + (pos - tmp->pos),tmp->length);
		pos = 0;
	}
}

str
readline_buffer(Buffer src, int pos)
{
	int eol = find_buffer(src,pos,"\r\n");
	return read_str(src,pos,eol-pos);
}

int
skipheaders_buffer(Buffer src, int pos)
{
	Buffer tmp;
	for (tmp = seek_buffer(src,pos); tmp; tmp = seek_buffer(src,pos)) {
		str line = readline_buffer(src,pos);
		pos += line->len + 2;
		if (line->len == 0) break;
	}
	return pos;
}

Buffer
dechunk_buffer(Buffer src)
{
	int delta = 0;
	Buffer tmp, retval = NULL;
	int pos = skipheaders_buffer(src,0);
	for (tmp = seek_buffer(src,pos); tmp; tmp = seek_buffer(src,pos)) {
		str line = readline_buffer(src,pos);	
		debug("Line is %s",line);
		delta = str_int(Str("0x%s",line));
		debug("Buffer Delta is %i",delta);
		retval = read_buffer(retval,src,pos+line->len+2,delta);
		pos += delta + line->len + 4;
		if (delta == 0) {
			debug("Done reading");
			return retval;
		}
	}
	return retval;
}
