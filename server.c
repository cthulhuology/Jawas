// jawas server.c
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "headers.h"
#include "index.h"
#include "status.h"
#include "events.h"
#include "files.h"
#include "sockets.h"
#include "signals.h"
#include "server.h"
#include "uri.h"
#include "usage.h"
#include "methods.h"
#include "lws.h"
#include "strings.h"
#include "sms.h"

Server server;

void
incoming(uint64_t fd)
{
	debug("Incoming %i",fd);
	server.socket = accept_socket(fd,(server.http_sock == fd ? NULL : server.tls));
	if (!server.socket) return;
	server.request = open_request(server.socket);
	add_read_socket(server.socket->fd,server.request);
}

void
disconnect()
{
	debug("Disconnect %p, %i",server.socket,server.socket->fd);
	close_socket(server.socket);
}

void
retry_request()
{
	debug("Retry Request %p, %i",server.request,server.request->retries);
	if (server.request->retries >= MAX_RETRIES)  {
		error("Failed to read request\n");
		disconnect();
		return;
	}
	add_read_socket(server.socket->fd,server.request);
}

void
read_request()
{
	debug("Read Request %p",server.request);
	if (process_request()) {
		retry_request();
		return;
	}
	server.request->retries = 0;
	if (!server.request->done) {
		add_read_socket(server.socket->fd,server.request);
		return;
	}
	debug("Read Request %p, %i done",server.request,server.socket->fd);
	server.socket->buf = NULL;
	server.response = new_response(server.request);
	parse_path(server.request);
	add_write_socket(server.socket->fd,server.response);
	return;
}


void
send_response()
{
	debug("Send response to %d",server.socket->fd);
	str host = parse_host(server.request,find_header(server.request->headers,$("Host")));
	debug("Connect to host %s",host);
	str method = parse_method(server.request);
	debug("Request method %s",method);
	begin_response();
	server.response->status = host && method ?
		dispatch_method(method) :
		error_handler(400);
	end_response();
	disconnect();
}

void
read_response()
{
	if (process_response()) {
		error("Failed to read response\n");
		disconnect();		
		return;
	}
	if (!server.response->done) {
		add_resp_socket(server.socket->fd,server.response);
		return;
	}
	Response tmp = server.response;
	server.request = server.request = server.response->request;
	server.socket = server.request->socket;
	server.response = server.request->response;
	append_header(server.response->headers,$("data"),from(tmp->contents,tmp->body,len(tmp->contents) - tmp->body));
	append_header(server.response->headers,$("status"),from(tmp->contents,9,3));
	close_socket(tmp->socket);
	connection(server.response->headers,"close");
	add_write_socket(server.response->socket->fd,server.response);
}

void
write_request()
{
	if (send_request(server.request)) {
		add_req_socket(server.socket->fd,server.request);
		return;
	}
	add_resp_socket(server.socket->fd,new_response(server.request));
}

void
serve(int port, int tls_port)
{
	init_regions();
	set_cwd();
	server.kq = kqueue();
	server.alarm = 0;
	server.http_sock = open_socket(port);
	server.tls_sock = open_socket(tls_port);
	server.tls = init_tls(TLS_KEYFILE,TLS_PASSWORD);
	server.tls_client = client_tls("certs");
	server.time = time(NULL);
	server.numevents = 2;
	monitor_socket(server.http_sock);
	monitor_socket(server.tls_sock);
	server.done = 0;
	general_signal_handlers();
	socket_signal_handlers();
	init_strings();
}

uint64_t
external_port(uint64_t fd)
{
	return fd == server.http_sock || fd == server.tls_sock;
}

void
use_event(Event e) {
	server.socket = e ? e->socket : NULL;
	server.request = e ? e->request : NULL;
	server.response = e ? e->response : NULL;
	server.file = e ? e->file : NULL;
}

void
run()
{
	Event e = poll_events();
	for (server.event = NULL; e; e = e->next) {
		if (!e->fd) continue;
		if (external_port(e->fd)) {
			incoming(e->fd);
			continue;
		}
		use_event(e);
		switch (e->type) {
		case READ:
			debug("Event READ");
			closed_socket(server.socket,"Read failed") ?
				disconnect():
				read_request();
			break;
		case WRITE:
			debug("Event WRITE");
			closed_socket(server.socket,"Write failed") ?
				disconnect() :
				send_response();
			break;
		case REQ:
			debug("Event REQ");
			closed_socket(server.socket,"Request failed") ?
				disconnect() :
				write_request();
			break;	
		case RESP:
			debug("Event RESP");
			closed_socket(server.socket,"Response failed") ?
				disconnect() :
				read_response();
			break;	
		case NODE:
			debug("Event NODE");
			reload(server.file);
			break;
		default:
			debug("UNKNOWN EVENT");
		}
	}
	if (server.done) {
		stop();
		exit(0);
	}
}

void
stop()
{
	close(server.http_sock);
	close(server.tls_sock);
	close(server.kq);
	close_sockets();
	close_files();
}
