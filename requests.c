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
#include "uri.h"
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
calc_chunked_length(Buffer buf)
{
	int total = 0;
	int pos = 0;
	int delta = 0;
	Buffer tmp;
	str line;

	for (tmp = seek_buffer(buf,pos); tmp; tmp = seek_buffer(buf,pos)) {
		line = readline_buffer(buf,pos);
		pos += line->len + 2;
		if (line->len == 0) break;
	}
	int body = pos;		
	for (tmp = seek_buffer(buf,pos); tmp; tmp = seek_buffer(buf,pos)) {
		line = readline_buffer(buf,pos);	
		debug("Line is %s",line);
		delta = str_int(Str("0x%s",line));
		total += delta;
		debug("Delta is %i",delta);
		pos += delta + line->len + 4;
		if (delta == 0) {
			debug("Done reading");
			return total;
		}
	}
	return 0x7fffffff;
}

Request
dechunk_request(Request req)
{

}

int
request_content_length(Request req)
{
	if (! req) return 0;
	str enc = find_header(req->headers, Transfer_Encoding_MSG);
	if (enc && icmp_str(enc,Str("chunked"))) {
		return calc_chunked_length(req->contents);
	}
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
			if (count > 2) {
				*body = o+1;
			//	debug("=== BODY ===");
			//	dump_buffer(buf,*body);
			//	debug("=== DONE ===");
				return headers;
			}
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
		//	debug("Settings headers %s = %s", headers[i].key, headers[i].value);
			reset = 0;
			o += l-1;
			++i;
		}
	}
	debug("***** BODY NOT SET!!!");
	return headers;
}

Request 
read_request(Request req)
{
	req->contents = read_socket(req->sc);
	if (!req->contents) {
		error("No request contents on request %i\n",req);
		return NULL;
	}
	if (!req->body) {
		req->headers = parse_request_headers(req->contents,&req->body);
		if (!req->headers) {
			error("No request headers on request %i\n",req);
			return NULL;
		}
	}
	if (req->body) {
		req->done = (length_buffer(req->contents) - req->body) >= request_content_length(req);
		debug("Request done %c [%i of %i bytes]", req->done ? "yes" : "no", length_buffer(req->contents), request_content_length(req));
	}
//	debug("REQUEST CONTENTS >>");
//	dump_buffer(req->contents,0);
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

str
parse_host()
{
	str host = find_header(Req->headers,"Host");
	if (! host && Sock->host) {
		debug("USING SOCKET HOST");
		host = Sock->host;
	}
	if (! host) return NULL;
	if (! Sock->host) Sock->host = host;
	return Req->host = host;
}

str
parse_method()
{
	char* retval = NULL;
	int i,l;
	Buffer tmp = seek_buffer(Req->contents,0);
	if (!tmp) return NULL;
	for (i=0;isspace(tmp->data[i]);++i);	// skip errorenous spaces
	for (l = 1; !isspace(tmp->data[i+l]); ++l);
	return read_str(tmp,i,l);	
}

str
parse_path()
{
	int i,l,end;
	Buffer qs;
	Buffer tmp = seek_buffer(Req->contents,0);
	if (! tmp) return NULL;
	for (i = 0;isspace(tmp->data[i]);++i);	// skip errorenous spaces
	for (i = 0; tmp->data[i] && !isspace(tmp->data[i]); ++i);
	for (;isspace(tmp->data[i]);++i);
	for (end = i; tmp->data[end] && !isspace(tmp->data[end]) && tmp->data[end] != '?'; ++end);
	Req->query_vars = NULL;
	if (tmp->data[end] == '?') {
		for (l = 1; !isspace(fetch_buffer(Req->contents,end + l)); ++l);
		qs = read_buffer(NULL,Req->contents,end,l);
		Req->query_vars = parse_uri_encoded(NULL,qs,1,length_buffer(qs));
		free_buffer(qs);
	}
	return Req->path = read_str(Req->contents,i,end - i);
}

