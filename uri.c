// uri.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "str.h"
#include "log.h"
#include "buffers.h"
#include "uri.h"
#include "server.h"

Headers
parse_uri_encoded(Headers head, Buffer buf, int pos, int len)
{
	if (!buf) return NULL;
	//debug("==== DUMPING BODY ====");
	//dump_buffer(buf,pos);
	//debug("==== LEN %i ====",len);
	int i,o;
	str key = NULL, value = NULL;
	Headers retval = (head ? head :  new_headers());
	for (i = pos; isspace(fetch_buffer(buf,i)); ++i);
	for (; i < len; ++i) {
		o = find_buffer(buf,i,"=");
		key = read_str(buf,i,o-i);
		i = o+1;
		o = find_buffer(buf,i,"&\r\n");
		value = read_str(buf,i,o-i);
		append_header(retval,key,value);
		i = o;
	}
	return retval;
}

int
is_mark_char(char c)
{
	int k;
	for (k = 0; k < mark_len; ++k) 
		if (mark_chars[k] == c) 
			return 1;
	return 0;
}

int
is_clean_char(char c)
{
	return (between('A',c,'Z')
		||  between('a',c,'z')
		||  between('0',c,'9')
		|| is_mark_char(c)); 
}

str
uri_encode(str s)
{
	str retval = NULL;
	int i, j = 0, len = s->len;
	for (i = 0; i < len; ++i)
		j += is_clean_char(s->data[i]) ? 1 : 3;
	retval = char_str(NULL,j);
	j = 0;
	for (i = 0; i < len; ++i) {
		if (is_clean_char(s->data[i])) {
			retval->data[j++] = s->data[i];
		} else {
			retval->data[j++] = '%';
			retval->data[j++] = hex_chars[s->data[i]/16];
			retval->data[j++] = hex_chars[s->data[i]%16];
		}
	}
	return retval;
}

char
from_hex(char a, char b)
{
	char retval = 0;
	if (between('0',a,'9')) retval += (a - '0')*16;
	if (between('A',a,'F')) retval += (a - 'A'+10)*16;
	if (between('a',a,'f')) retval += (a - 'a'+10)*16;
	if (between('0',b,'9')) retval += (b - '0');
	if (between('A',b,'F')) retval += (b - 'A'+10);
	if (between('a',b,'f')) retval += (b - 'a'+10);
	return retval;
}


str
uri_decode(str s)
{
	int i, j = 0, len = s->len;
	str retval = char_str(NULL,len);
	for(i = 0; i < len; ++i)
		if (s->data[i] == '+') {
			retval->data[j++] = ' ';
		} else if (s->data[i] == '%') {
			retval->data[j++] = from_hex(s->data[i+1],s->data[i+2]);
			i += 2;
		} else {
			retval->data[j++] = s->data[i];
		}
	retval->data[j] = '\0';
	retval->len = j;
	return retval;
}
