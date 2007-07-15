// uri.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "buffers.h"
#include "uri.h"

Headers
parse_query_string(Buffer buf)
{
	char c;
	int i,l,len;
	Headers retval = new_headers();
	Buffer key, value;
	if (!buf) return NULL;
	fprintf(stderr, "Query String: ");
	print_buffer(buf);
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
	dump_headers(retval);
	return retval;
}

Buffer
parse_path(Request req)
{
	int i,l,end;
	Buffer qs;
	Buffer tmp = seek_buffer(req->contents,0);
	if (! tmp) return NULL;
	for (i = 0; tmp->data[i] && !isspace(req->contents->data[i]); ++i);
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
	if (! host) fprintf(stderr, "FAILED TO LOCATE HOST IN  HEADER!!!\n");
	else fprintf(stderr, "Host is %d %s\n",host->length, host->data);
	return req->host =  host ? host : write_buffer(NULL,"localhost",9);
}

char*
file_path(Request req,char* filename,int flen)
{
	size_t cwdlen;
	while (! req->host) parse_host(req);
	char* cwd = getcwd(NULL,0);
	cwdlen = strlen(cwd);
	char* retval = (char*)malloc(cwdlen + req->host->length + flen + 3);
	memset(retval,0,cwdlen + req->host->length + flen + 3);
	memcpy(retval,cwd,strlen(cwd));
	memcpy(retval + cwdlen,"/",1);
	memcpy(retval + cwdlen + 1, req->host->data, req->host->length);
	memcpy(retval + cwdlen + 1 + req->host->length, filename, flen);
	free(cwd);
	fprintf(stderr, "file_path [%s]\n",retval);
	return retval;	
}

char*
request_path(Request req)
{
	while (! req->path) parse_path(req);	
	return file_path(req,req->path->data,req->path->length);
}

int
is_mark_char(char c)
{
	int k;
	for (k = 0; k < mark_len; ++k) if (mark_chars[k] == c) return 1;
	return 0;
}

Buffer
uri_encode(Buffer buf)
{
	Buffer retval = new_buffer(NULL,0);
	char c;
	int i,j,len = length_buffer(buf);
	j = 0;
	for (i = 0; i < len; ++i) {
		c = fetch_buffer(buf,i);
		if (c >= 'A' && c <= 'Z' 
		||  c >= 'a' && c <= 'z'
		||  c >= '0' && c <= '9'
		|| is_mark_char(c)) {
			retval->data[j] = c;
			++retval->length; 
			if (j % Max_Buffer_Size == Max_Buffer_Size - 1) retval = new_buffer(retval,j+1);
			++j;		
		} else {
			retval->data[j % Max_Buffer_Size] = '%';
			++retval->length; 
			if (j % Max_Buffer_Size == Max_Buffer_Size - 1) retval = new_buffer(retval,j+1);
			++j;
			retval->data[j] = hex_chars[c/16];
			++retval->length; 
			if (j % Max_Buffer_Size == Max_Buffer_Size - 1) retval = new_buffer(retval,j+1);
			++j;
			retval->data[j] = hex_chars[c%16];
			++retval->length; 
			if (j % Max_Buffer_Size == Max_Buffer_Size - 1) retval = new_buffer(retval,j+1);
			++j;	
		}
	}
	return retval;
}

char
from_hex(char a, char b)
{
	char retval = 0;
	if (a >= '0' && a <= '9') retval += (a - '0')*16;
	if (a >= 'A' && a <= 'F') retval += (a - 'A'+10)*16;
	if (a >= 'a' && a <= 'f') retval += (a - 'a'+10)*16;
	if (b >= '0' && b <= '9') retval += (b - '0');
	if (b >= 'A' && b <= 'F') retval += (b - 'A'+10);
	if (b >= 'a' && b <= 'f') retval += (b - 'a'+10);
	return retval;
}


Buffer
uri_decode(Buffer buf)
{
	Buffer retval = new_buffer(NULL,0);
	char c;
	int i,j, len = length_buffer(buf);
	j = 0;	
	for(i = 0; i < len; ++i) {
		c = fetch_buffer(buf,i);
		if (c == '%') {
			retval->data[j % Max_Buffer_Size] = from_hex(fetch_buffer(buf,i+1),fetch_buffer(buf,i+2));
			++retval->length; 
			i += 2;
			if (j % Max_Buffer_Size == Max_Buffer_Size - 1) retval = new_buffer(retval,j+1);
			++j;
		} else {
			retval->data[j % Max_Buffer_Size] = c;
			++retval->length; 
			if (j % Max_Buffer_Size == Max_Buffer_Size - 1) retval = new_buffer(retval,j+1);
			++j;
		}
	}
	fprintf(stderr,"[uri_decode] %s\n",retval->data);
	return retval;
}
