// uri.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "log.h"
#include "buffers.h"
#include "uri.h"

str cwd = NULL;

Headers
parse_query_string(Buffer buf)
{
	return parse_uri_encoded(NULL,buf,1,length_buffer(buf));
}

Headers
parse_post_request(Request req)
{
	return parse_uri_encoded(req->query_vars,req->contents,req->body,length_buffer(req->contents));
}

Headers
parse_uri_encoded(Headers head, Buffer buf, int pos, int len)
{
	if (!buf) return NULL;
	int i,o;
	str key = NULL, value = NULL;
	Headers retval = (head ? head :  new_headers());
	for (i = pos; isspace(fetch_buffer(buf,i)); ++i);
	for (; i < len; ++i) {
		o = find_buffer(buf,i,"=");
		key = read_str(buf,i,o-i);
		i = o+1;
		o = find_buffer(buf,i,"&");
		value = read_str(buf,i,o-i);
		append_header(retval,key,value);
		i = o;
		key = NULL;
		value = NULL;
	}
	return retval;
}

str
parse_path(Request req)
{
	int i,l,end;
	Buffer qs;
	Buffer tmp = seek_buffer(req->contents,0);
	if (! tmp) return NULL;
	for (;isspace(tmp->data[i]);++i);	// skip errorenous spaces
	for (i = 0; tmp->data[i] && !isspace(tmp->data[i]); ++i);
	for (;isspace(tmp->data[i]);++i);
	for (end = i; tmp->data[end] && !isspace(tmp->data[end]) && tmp->data[end] != '?'; ++end);
	req->query_vars = NULL;
	if (tmp->data[end] == '?') {
		for (l = 1; !isspace(fetch_buffer(req->contents,end + l)); ++l);
		qs = read_buffer(NULL,req->contents,end,l);
		req->query_vars = parse_query_string(qs);
		free_buffer(qs);
	}
	return req->path = read_str(req->contents,i,end - i);
}

str
parse_host(Request req)
{
	str host = find_header(req->headers,"Host");
	if (! host && req->sc->host) {
		debug("USING SOCKET HOST");
		host = req->sc->host;
	}
	if (! host) return NULL;
	if (! req->sc->host) req->sc->host = host;
	req->host = host;
	notice("Host is %i %s\n",req->host->len, req->host);
	return req->host;
}

str
parse_method(Request req)
{
	char* retval = NULL;
	int i,l;
	Buffer tmp = seek_buffer(req->contents,0);
	if (!tmp) return NULL;
	for (i=0;isspace(tmp->data[i]);++i);	// skip errorenous spaces
	for (l = 1; !isspace(tmp->data[i+l]); ++l);
	return read_str(tmp,i,l);	
}

str
file_path(str host,str filename)
{
	if (!cwd)
		cwd = char_str(getcwd(NULL,0),0);
	return Str("%s/%s%s",cwd,host,filename);
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

char*
uri_encode(char* str)
{
	char* retval = NULL;
	int i, j = 0, len = strlen(str);
	for (i = 0; i < len; ++i)
		j += is_clean_char(str[i]) ? 1 : 3;
	retval = salloc(j);
	j = 0;
	for (i = 0; i < len; ++i) {
		if (is_clean_char(str[i])) {
			retval[j++] = str[i];
			++j;
		} else {
			retval[j++] = '%';
			retval[j++] = hex_chars[str[i]/16];
			retval[j++] = hex_chars[str[i]%16];
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


char*
uri_decode(char* str)
{
	int i, j = 0, len = strlen(str);
	char* retval = salloc(len);
	for(i = 0; i < len; ++i) {
		if (str[i] == '%') {
			retval[j++] = from_hex(str[i+1],str[i+2]);
			i += 2;
		} else {
			retval[j++] = str[i];
		}
	}
	return retval;
}
