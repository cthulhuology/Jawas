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

