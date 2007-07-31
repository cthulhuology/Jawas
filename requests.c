// requests.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "log.h"
#include "headers.h"
#include "events.h"
#include "requests.h"
#include "server.h"

Request
open_request(Socket sc)
{
	Request retval = (Request)salloc(sizeof(struct request_struct));
	if (retval) {
		memset(retval,0,sizeof(struct request_struct));
		retval->sc = sc;
	}
	return retval;
}

int
request_content_length(Request req)
{
	Buffer value;
	if (! req) return 0;
	value = find_header(req->headers, Content_Length_MSG);
	return value ? strtol(value->data,NULL,0) : 0;
}

Headers
parse_request_headers(Buffer buf, int* body)
{
	char c;
	int i,o,l,reset,count;
	int len = length_buffer(buf);
	Headers headers = new_headers();
	if (! headers) {
		error("Failed to allocate new headers\n");
		return NULL;
	}
	count = 0;
	for (o = 0; o < len; ++o ) {
		c = fetch_buffer(buf,o);
		if (c == '\r' || c == '\n') break;
	}
	if (o >= len) {
		error("No line breaks found! Bad request\n");
		return NULL;
	}
	for (i = 0; i < MAX_HEADERS && o < len; ++o) {
		c = fetch_buffer(buf,o);
		if (c == '\r' || c == '\n') {
			reset = 1;
			++count;
			debug("Count is %i",count);
			if (count > 2) break;
			continue;
		}
		count = 0;
		if (reset && ! headers[i].key) {
			for (l = 1; (o + l) < len && fetch_buffer(buf,o+l) != ':'; ++l);
			headers[i].key = read_buffer(NULL,buf,o,l);
			o += l-1;
			c = fetch_buffer(buf,o);
			debug("Headers [%s:]",headers[i].key->data);
		} 
		if (reset && c == ':') {
			o += 1;
			while(isspace(c = fetch_buffer(buf,o))) ++o;
			for (l = 1; (o + l) < len && c != '\r' && c != '\n'; ++l) c = fetch_buffer(buf,o+l); 
			headers[i].value = read_buffer(NULL,buf,o,l-1);
			debug("Headers [%s:%s]",headers[i].key->data,headers[i].value->data);
			reset = 0;
			o += l-1;
			++i;
		}
	}
	*body = o;
	return headers;
}

Headers
parse_post_request(Request req)
{
	Buffer key = NULL, value;
	char c;
	int i,o,l,len = length_buffer(req->contents);
	if (!req->query_vars) 
		req->query_vars = new_headers();
	debug("PARSE_POST_REQUEST %i through %i", req->body,len);
	for (o = req->body; isspace(fetch_buffer(req->contents,o)); ++o);	
	for (; o < len; ++o) {
		c = fetch_buffer(req->contents,o); 
		if (key == NULL) {
			for (l = 1; o+l < len && '=' != fetch_buffer(req->contents,o+l); ++l);
			key = read_buffer(NULL,req->contents,o,l);	
			o += l - 1;
		}
		if (c == '=' ) {
			++o;
			for (l = 1; o+l < len && '&' != fetch_buffer(req->contents,o+l); ++l);
			value = read_buffer(NULL,req->contents,o,l);	
			debug("POST: %s = %s",key->data,value->data);
			append_header(req->query_vars,key,value);
			key = NULL;
			o += l;
		}
	}
	debug("PARSE_POST_DONE");
	return req->headers;
}

Request 
read_request(Request req)
{
	int tmp,hdrs;
	req->contents = read_socket(req->sc);
	if (!req->contents) {
		error("No request contents on request %i\n",req);
		return NULL;
	}
	req->headers = parse_request_headers(req->contents,&req->body);
	debug("Request body at offset %i",req->body);
	if (!req->headers) {
		error("No request headers on request %i\n",req);
		return NULL;
	}
	req->done = (length_buffer(req->contents) - req->body) >= request_content_length(req);
	return req;
}

void 
close_request(Request req)
{
	Buffer buf;
	if (!req) return;
	free_buffer(req->path);
	// NB: don't free_buffer(req->host) because it is freed below as part of headers
	for (buf = req->contents; buf; buf = free_buffer(buf));
	if (req->query_vars) free_headers(req->query_vars);
	free_headers(req->headers);
}

