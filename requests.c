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
	if (! req) return 0;
	str value = find_header(req->headers, Content_Length_MSG);
	return str_int(value);
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
			if (count > 2) break;
			continue;
		}
		count = 0;
		if (reset && ! headers[i].key) {
			for (l = 1; (o + l) < len && fetch_buffer(buf,o+l) != ':'; ++l);
			headers[i].key = read_str(buf,o,l);
			o += l-1;
			c = fetch_buffer(buf,o);
		} 
		if (reset && c == ':') {
			o += 1;
			while(isspace(c = fetch_buffer(buf,o))) ++o;
			for (l = 1; (o + l) < len && c != '\r' && c != '\n'; ++l) c = fetch_buffer(buf,o+l); 
			headers[i].value = read_str(buf,o,l-1);
			reset = 0;
			o += l-1;
			++i;
		}
	}
	*body = o;
	return headers;
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
	for (buf = req->contents; buf; buf = free_buffer(buf));
	if (req->query_vars) free_headers(req->query_vars);
	free_headers(req->headers);
}

