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
		retval->usage = new_usage(0);
	}
	return retval;
}

int
is_chunked(Request req)
{
	str enc = find_header(req->headers, Transfer_Encoding_MSG);
 	// debug("Encoding: %s",enc);
 	return enc && icmp_str(enc,Str("chunked"));
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
		if (line->len == 0) break;
	// 	debug("Line is %s",line);
		delta = str_int(Str("0x%s",line));
		total += delta;
		pos += delta + line->len + 4;
		if (delta == 0) {
			return total;
		}
	}
	return 0x7fffffff;
}

Request
dechunk_request(Request req)
{
	if (is_chunked(req)) {
		Buffer hed = read_buffer(NULL,req->contents,0,req->body);
		Buffer con = dechunk_buffer(req->contents);
		req->contents = read_buffer(hed,con,0,length_buffer(con));
	}
	return req;
}

int
request_content_length(Request req)
{
	if (! req) return 0;
	str enc = find_header(req->headers, Transfer_Encoding_MSG);
	if (is_chunked(req)) 
		return calc_chunked_length(req->contents);
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
				//debug("=== BODY ===");
				//dump_buffer(buf,*body);
				//debug("=== DONE ===");
				return headers;
			}
			continue;
		}
		count = 0;
		if (reset && ! Key(headers,i)) {
			for (l = 1; (o + l) < len && fetch_buffer(buf,o+l) != ':'; ++l);
			headers->nslots++;
			headers->slots[i].key = read_str(buf,o,l);
			o += l-1;
			c = fetch_buffer(buf,o);
		}
		if (reset && c == ':') {
			o += 1;
			while(isspace(c = fetch_buffer(buf,o))) ++o;
			for (l = 1; (o + l) < len && c != '\r' && c != '\n'; ++l) c = fetch_buffer(buf,o+l); 
			headers->slots[i].value = read_str(buf,o,l-1);
			debug("Headers[%i] [%s=%s]",i,Key(headers,i),Value(headers,i));
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
// 		debug("Request done %c [%i of %i bytes]", req->done ? "yes" : "no", length_buffer(req->contents), request_content_length(req));
	}
	if (req->done) return dechunk_request(req);
	//debug("REQUEST CONTENTS >>");
//	dump_buffer(req->contents,0);
	return req;
}

void 
close_request(Request req)
{
	Buffer buf;
	if (!req) return;
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
	}
	return Req->path = read_str(Req->contents,i,end - i);
}

RequestInfo
start_request(RequestInfo ri, Request req) {
	start_usage(req->usage);
	return ri;
}

RequestInfo
end_request(RequestInfo ri, Request req) {
	RequestInfo tmp;
	// debug("Ending request %p",req);
	stop_usage(req->usage);
	for (tmp = ri; tmp; tmp = tmp->next) {
		if(cmp_str(tmp->host,req->host))
			if (cmp_str(tmp->path,req->path)) {
				++tmp->hits;
				tmp->time = (req->usage->time + (tmp->hits-1) * tmp->time) / tmp->hits;
				return ri;
			}
	}
	server_scratch();
	tmp = (RequestInfo)salloc(sizeof(struct request_info_struct));	
	tmp->next = ri;
	tmp->host = Str("%s",req->host);
	tmp->path = Str("%s",req->path);
	tmp->time = req->usage->time;
	tmp->hits = req->usage->hits;
	old_scratch();
	return tmp;
}
