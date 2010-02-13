// uri.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "str.h"
#include "log.h"
#include "uri.h"
#include "server.h"

static char mark_chars[] = "-_.!~*'()";
static int mark_len = 9;
static char hex_chars[] = "0123456789ABCDEF";

Headers
parse_uri_encoded(Headers head, str buf, int pos)
{
	if (!buf) return NULL;
	int i,o,l = len(buf);
	str key = NULL, value = NULL;
	Headers retval = (head ? head :  new_headers());
	for (i = pos; isspace(at(buf,i)); ++i);
	for (; i < l; ++i) {
		o = find(buf,i,"=",1);
		if (o >= l) {
			 debug("Key not found %i > %i",o,l);
			 break;
		}
		key = ref(buf->data+i,o-i);
		i = o+1;
		o = find(buf,i,"&\r\n",1);
		value = (o > l) ?
			ref(buf->data+i,l-i):
			ref(buf->data+i,o-i);
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
	int i, j = 0, l = len(s);
	for (i = 0; i < l; ++i)
		j += is_clean_char(at(s,i)) ? 1 : 3;
	retval = blank(j);
	j = 0;
	for (i = 0; i < l; ++i) {
		char c = at(s,i);
		if (is_clean_char(c)) {
			set(retval,j++,c);
		} else {
			set(retval,j++,'%');
			set(retval,j++,hex_chars[c/16]);
			set(retval,j++,hex_chars[c%16]);
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
	int i, j = 0, l = len(s);
	str retval = blank(l);
	for(i = 0; i < l; ++i)
		if (at(s,i) == '+') {
			set(retval,j++,' ');
		} else if (at(s,i) == '%') {
			set(retval,j++,from_hex(at(s,i+1),at(s,i+2)));
			i += 2;
		} else {
			set(retval,j++,at(s,i));
		}
	retval->length = j;
	return retval;
}
