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
#include "transfer.h"

Request
new_request(str method, str host, str path)
{
	Request retval = (Request)salloc(sizeof(struct request_struct));
	if (retval) {
		retval->method = clone(method);
		retval->host = name_field(host);	
		int port = str_int(skip_fields(host,0));	
		retval->port = port ? port : 80;
		retval->path = clone(path);
		retval->sc = NULL;
		retval->usage = new_usage(0);
		retval->headers = new_headers();
		retval->query_vars = new_headers();
		retval->contents = NULL;
		retval->resp = NULL;
		retval->cb = NULL;
		retval->body = 0;
		retval->done = 0;
		retval->written = 0;
		retval->length = -1;
	}
	return retval;
}

Request
request_port(Request req,int port)
{
	req->port = port;	
	return req;
}

Request
request_headers(Request req, str key, str value)
{
	append_header(req->headers,key,value);
	return req;
}

Request
request_data(Request req, str text)
{
	if (len(text) > 0) 
		req->contents = append(req->contents,text);
	return req;
}

Request
open_request(Socket sc)
{
	Request retval = new_request(NULL,NULL,NULL);
	if (retval) retval->sc = sc;
	return retval;
}

Request
dechunk_request(Request req)
{
	if (is_chunked(req->headers)) {
		str hed = from(req->contents,0,req->body);
		str con = dechunk(req->contents);
		req->contents = append(hed,con);
	}
	return req;
}

Request 
process_request(Request req)
{
	req->contents = read_socket(req->sc);
	if (!req->contents) {
		error("No request contents on request %i\n",req);
		return NULL;
	}
	if (!req->body) {
		req->headers = parse_headers(req->contents,&req->body);
		if (!req->headers) {
			error("No request headers on request %i\n",req);
			return NULL;
		}
	}
	if (req->body) {
		req->done = (len(req->contents) - req->body) >= inbound_content_length(req->contents,req->headers);
	}
	return req->done ? dechunk_request(req) : req;
}

str
parse_host()
{
	str host = find_header(Req->headers,Str("Host"));
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
	int i,l;
	str tmp = seek(Req->contents,0);
	if (!tmp) return NULL;
	for (i=0;isspace(at(tmp,i));++i);	// skip errorenous spaces
	for (l = 1; !isspace(at(tmp,i+l)); ++l);
	return from(tmp,i,l);
}

str
parse_path()
{
	int i,l,end;
	str qs;
	str tmp = seek(Req->contents,0);
	if (! tmp) return NULL;
	for (i = 0;isspace(at(tmp,i));++i);	// skip errorenous spaces
	for (i = 0; at(tmp,i) && !isspace(at(tmp,i)); ++i);
	for (;isspace(at(tmp,i));++i);
	for (end = i; at(tmp,end) && !isspace(at(tmp,end)) && at(tmp,end) != '?'; ++end);
	Req->query_vars = NULL;
	if (at(tmp,end) == '?') {
		for (l = 1; !isspace(at(tmp,end + l)); ++l);
		qs = from(tmp,end,l);
		Req->query_vars = parse_uri_encoded(NULL,qs,1);
	}
	return Req->path = from(tmp,i,end - i);
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
		if(cmp(tmp->host,req->host))
			if (cmp(tmp->path,req->path)) {
				++tmp->hits;
				tmp->time = (req->usage->time + (tmp->hits-1) * tmp->time) / tmp->hits;
				return ri;
			}
	}
	server_scratch();
	tmp = (RequestInfo)salloc(sizeof(struct request_info_struct));	
	tmp->next = ri;
	tmp->host = clone(req->host);
	tmp->path = clone(req->path);
	tmp->time = req->usage->time;
	tmp->hits = req->usage->hits;
	old_scratch();
	return tmp;
}

int
send_request(Request req)
{
	debug("send_request");
	if (! req->sc ) {
		req->sc = connect_socket(req->host->data,req->port);
		if (! req->sc) {
			error("Failed to connect to %s:%i\n",req->host,req->port);
			return 0;
		}
		debug("send_request connected, scheduling write");
		add_req_socket(req->sc->fd,req);
		return 0;
	}
	if (req->length < 0) {
		debug("send_request sending request");
		str cmd = Str("%s %s HTTP/1.1\r\n",req->method,req->path);
		debug("Fetching: %s",cmd);
		write_socket(req->sc,cmd);
		request_headers(req,Str("Host"),req->host);
		request_headers(req,Str("Transfer-Encoding"),Str("chunked"));
		send_headers(req->sc,req->headers);
		req->length = outbound_content_length(req->contents,NULL);	
		debug("send_request contents? %i", req->contents != NULL);
		if (req->contents == NULL) 	
			write_chunk(req->sc,NULL,0);
		return req->contents != NULL;
	}
	if (req->contents) {
		debug("send_request sending contents");
		req->written += send_contents(req->sc,req->contents,1);
	}
	if (req->written >= req->length)
		write_chunk(req->sc,NULL,0);
	debug("send_request more? %i", req->written < req->length);
	return req->written < req->length;
}

void
request_callback(Request req, Response resp,  str cb)
{
	req->resp = resp;
	req->cb = cb;
}
