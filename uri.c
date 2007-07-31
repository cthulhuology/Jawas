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

char* cwd;
int cwdlen;

Headers
parse_query_string(Buffer buf)
{
	char c;
	int i,l,len;
	Headers retval = new_headers();
	Buffer key, value;
	if (!buf) return NULL;
	len = length_buffer(buf);
	for (i = 0; i < len; ++i) {
		c = fetch_buffer(buf,i);
		if (c == '?' || c == '&') {
			++i;
			key = NULL;
			for (l = 1; i+l < len; ++l) {
				c = fetch_buffer(buf,i+l);
				if ('=' == c) {
					key = read_buffer(NULL,buf,i,l);
					i += l;
					break;
				}
			}
		}
		c = fetch_buffer(buf,i);
		if (c == '=') {
			++i;
			value = NULL;
			for (l = 1; i+l < len; ++l) {
				c = fetch_buffer(buf,i+l);
				if ('&' == c || (i+l+1 == len)) {
					if ('&' != c) ++l;
					value = read_buffer(NULL,buf,i,l);
					i += l-1;
					append_header(retval,key,value);
					break;
				}
			}
		}
	}
	return retval;
}

Buffer
parse_path(Request req)
{
	int i,l,end;
	Buffer qs;
	Buffer tmp = seek_buffer(req->contents,0);
	if (! tmp) return NULL;
	debug("REQUEST: %s",tmp->data);
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
	return req->path = read_buffer(NULL,req->contents,i,end - i);
}

Buffer
parse_host(Request req)
{
	Buffer host = find_header(req->headers,"Host");
	if (! host && req->sc->host) {
		debug("USING SOCKET HOST");
		host = write_buffer(NULL,req->sc->host,strlen(req->sc->host));
	}
	if (! host) return NULL;
	if (! req->sc->host) set_host(req->sc,host);
	req->host = host;
	notice("Host is %i %s\n",req->host->length, req->host->data);
	return req->host;
}

char*
parse_method(Request req)
{
	char* retval = NULL;
	int i,l;
	Buffer tmp = seek_buffer(req->contents,0);
	if (!tmp) return NULL;
	for (i=0;isspace(tmp->data[i]);++i);	// skip errorenous spaces
	for (l = 1; !isspace(tmp->data[i+l]); ++l);
	retval = salloc(l+1);
	memcpy(retval,&tmp->data[i],l);
	retval[l] = '\0';
	notice("Method is: %s",retval);
	return retval;
}

char*
file_path(char* host,int hlen, char* filename,int flen)
{
	if (!cwd) {
		cwd = getcwd(NULL,0);
		cwdlen = strlen(cwd);
	}
	char* retval = (char*)salloc(cwdlen + hlen + flen + 3);
	memset(retval,0,cwdlen + hlen + flen + 3);
	memcpy(retval,cwd,cwdlen);
	memcpy(retval + cwdlen,"/",1);
	memcpy(retval + cwdlen + 1, host, hlen);
	memcpy(retval + cwdlen + 1 + hlen, filename, flen);
	return retval;	
}

char*
request_path(Request req)
{
	return file_path(req->host->data,req->host->length,req->path->data,req->path->length);
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
