// requests.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "log.h"
#include "headers.h"
#include "events.h"
#include "requests.h"
#include "uri.h"
#include "server.h"
#include "transfer.h"

Request requests;

Request
new_request(str method, str host, str path)
{
	Request retval = (Request)reserve(sizeof(struct request_struct));
	if (!retval) return NULL;
	retval->method = method;
	retval->host = NULL;
	retval->port = 0;
	parse_host(retval,host);
	retval->path = path;
	retval->socket = NULL;
	retval->usage = new_usage(0);
	retval->headers = new_headers();
	retval->query_vars = new_headers();
	retval->contents = NULL;
	retval->raw_contents = NULL;
	retval->response = NULL;
	retval->cb = NULL;
	retval->body = 0;
	retval->done = 0;
	retval->written = 0;
	retval->length = -1;
	retval->retries = 0;
	retval->ssl = 0;
	return retval;
}

void
request_port(Request req, int port)
{
	req->port = port;	
}

void
request_headers(Request req, str key, str value)
{
	append_header(req->headers,key,value);
}

void
request_data(Request req, str text)
{
	if (len(text) > 0) req->contents = append(req->contents,text);
}

void
request_file(Request req, File fc)
{
	req->raw_contents = fc;
}

Request
open_request(Socket sc)
{
	Request retval = new_request(NULL,NULL,NULL);
	if (retval) retval->socket = sc;
	return retval;
}

void
dechunk_request()
{
	if (!is_chunked(server.request->headers)) return;
	str hed = from(server.request->contents,0,server.request->body);
	str con = dechunk(server.request->contents);
	server.request->contents = append(hed,con);
}

int 
process_request()
{
	server.request->contents = read_socket(server.request->socket);
	if (!server.request->contents) {
		error("No request contents on request %i\n",server.request);
		++server.request->retries;
		debug("retries %i",server.request->retries);
		return -1;
	}
	if (!server.request->body) {
		server.request->headers = parse_headers(server.request->contents,&server.request->body);
		debug("Headers:\n%s",print_headers(NULL,server.request->headers));
		if (!server.request->headers) {
			error("No request headers on request %i\n",server.request);
			return -1;
		}
		request_headers(server.request, _("peer"),socket_peer(server.request->socket));
	}
	if (server.request->body) {
		server.request->done = (len(server.request->contents) - server.request->body) >= inbound_content_length(server.request->contents,server.request->headers);
	}
	if (server.request->done) dechunk_request();
	return 0;
}

void
request_ssl(Request req)
{
	debug("Setting request to SSL mode");
	req->ssl = 1;
}

str
parse_host(Request req, str host)
{
	if (!req) return NULL;
	if (! host) {
		req->port = 0;
		return req->host = NULL;
	}
	req->host = name_field(host);
	int port = str_int(skip_fields(host,0));
	request_port(req,port ? port : 80);
	return req->host;
}

str
parse_method()
{
	int i,l;
	str tmp = server.request->contents;
	if (!tmp) return NULL;
	for (i=0;isspace(at(tmp,i));++i);	// skip errorenous spaces
	for (l = 1; !isspace(at(tmp,i+l)); ++l);
	server.request->method = from(tmp,i,l);
	return server.request->method;
}

str
parse_path()
{
	int i,l,end;
	str qs;
	str tmp = server.request->contents;
	if (! tmp) return NULL;
	for (i = 0;isspace(at(tmp,i));++i);	// skip errorenous spaces
	for (i = 0; at(tmp,i) && !isspace(at(tmp,i)); ++i);
	for (;isspace(at(tmp,i));++i);
	for (end = i; at(tmp,end) && !isspace(at(tmp,end)) && at(tmp,end) != '?'; ++end);
	server.request->query_vars = NULL;
	if (at(tmp,end) == '?') {
		for (l = 1; !isspace(at(tmp,end + l)); ++l);
		qs = from(tmp,end,l);
		server.request->query_vars = parse_uri_encoded(NULL,qs,1);
	}
	return server.request->path = from(tmp,i,end - i);
}

RequestInfo
start_request(RequestInfo ri, Request req) {
	start_usage(req->usage);
	return ri;
}

RequestInfo
end_request(RequestInfo ri, Request req) {
	RequestInfo tmp;
	stop_usage(req->usage);
	debug("Request %s:%i%s",req->host,req->port,req->path);
	dump_usage(req->usage);
	for (tmp = ri; tmp; tmp = tmp->next) {
		if(cmp(tmp->host,req->host))
			if (cmp(tmp->path,req->path)) {
				++tmp->hits;
				tmp->time = (req->usage->time + (tmp->hits-1) * tmp->time) / tmp->hits;
				return ri;
			}
	}
	tmp = (RequestInfo)reserve(sizeof(struct request_info_struct));	
	tmp->next = ri;
	tmp->host = req->host;
	tmp->path = req->path;
	tmp->time = req->usage->time;
	tmp->hits = req->usage->hits;
	return tmp;
}

int
send_request(Request req)
{
	if (! req->socket ) {
		req->socket = connect_socket(req->host,req->port,0);
		if (! req->socket) {
			error("Failed to connect to %s:%i\n",req->host,req->port);
			return 0;
		}
		add_req_socket(req->socket->fd,req);
		return 0;
	}
	if (req->length < 0) {
		str cmd = _("%s %s HTTP/1.1\r\n",req->method,req->path);
		write_socket(req->socket,cmd);
		request_headers(req,_("Host"),req->host);
		send_headers(req->socket,req->headers);
		req->length = outbound_content_length(req->contents,req->raw_contents);	
		return req->contents != NULL || req->raw_contents != NULL ;
	}
	req->written += req->contents ?
			send_contents(req->socket,req->contents,is_chunked(req->headers)) :
		req->raw_contents ?
			send_raw_contents(req->socket,req->raw_contents,req->written,0):
			0;
	if (is_chunked(req->headers) && req->written >= req->length)
		write_chunk(req->socket,NULL,0);
	return req->written < req->length;
}

void
request_callback(Request req, Response resp,  str cb)
{
	req->response = resp;
	req->cb = cb;
}
