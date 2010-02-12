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

Server srv = NULL;

File
load(str filename)
{
	File retval = NULL;
	if (!filename) return NULL;
	retval = query_cache(&srv->fc,filename);
	if (retval) return retval;
	retval = open_file(srv->fc,filename);
	if (!retval) {
		error("Failed to open %s\n",filename);
		return NULL;
	}
	srv->fc = retval;
	add_file_monitor(srv->fc->fd,srv->fc);
	return retval;
}

void
unload(int fd, str filename)
{
	srv->fc = close_file(srv->fc,filename);
}

void
resume(Socket sc)
{
	Request req;
	req = open_request(srv->sc);
	srv->ri = start_request(srv->ri,req);
	add_read_socket(srv->sc->fd,req);
}

void
incoming(int fd)
{
	srv->sc = accept_socket(srv->sc,fd,(srv->http_sock == fd ? NULL : srv->tls));
	if (srv->sc) resume(srv->sc);
}

void
disconnect()
{
	Socket tmp,last;
	last = NULL;
	for (tmp = srv->sc; tmp; tmp = tmp->next) {
		if (tmp == Sock)
			last ? (last->next = close_socket(Sock)):
			(srv->sc = close_socket(Sock));
		last = tmp;
	}
}

void
read_request()
{
	if (!process_request(Req)) {
		if (Req->retries < MAX_RETRIES)  {
			add_read_socket(Sock->fd,Req);
			return;
		}
		error("Failed to read request\n");
		disconnect();
		return;
	}
	Req->retries = 0;
	if (Req->done) {
		Sock->buf = NULL;
		Resp = new_response(Req);
		parse_path(Req);	
		add_write_socket(Sock->fd,Resp);
	} else {
		add_read_socket(Sock->fd,Req);
	}
}


void
send_response()
{
	str host = parse_host(Req,find_header(Req->headers,$("Host")));
	str method = parse_method(Req);
	begin_response(Resp);
	Resp->status = host && method ?
		dispatch_method(method) :
		error_handler(400);
	end_response(Resp);
}


void
read_response()
{
	if (!process_response(Resp)) {
		error("Failed to read response\n");
		disconnect();		
		return;
	}
	if (Resp->done) {
		Response tmp = Resp;
		Headers hdrs = tmp->headers;
		append_header(hdrs,$("data"),from(tmp->contents,tmp->body,len(tmp->contents) - tmp->body));
		append_header(hdrs,$("status"),from(tmp->contents,9,3));
		set_SockReqResp(NULL,NULL,Resp->req->resp);
		connection(Resp->headers,"close");
		close_socket(tmp->sc);
		add_write_socket(Resp->sc->fd,Resp);
	} else {
		add_resp_socket(Sock->fd,Resp);
	}
}

void
write_response()
{
	send_response(Resp);
	srv->ri = end_request(srv->ri,Resp->req);
	disconnect();
}

void
write_request()
{
	if (send_request(Req)) {
		add_req_socket(Sock->fd,Req);
		return;
	}
	add_resp_socket(Sock->fd,new_response(Req));
}

str
load_config(char* filename)
{
	File conf = open_file(srv->fc,$("%c",filename));
	if (conf) {
		srv->fc = conf;
		return copy(conf->data,conf->st.st_size-1);		
	}
	return NULL;
}

void
serve(int port, int tls_port)
{
	init_regions();
	srv = (Server)system_reserve(sizeof(struct server_struct));
	set_cwd();
	srv->kq = kqueue();
	srv->alarm = 0;
	srv->http_sock = open_socket(port);
	srv->tls_sock = open_socket(tls_port);
	srv->tls = init_tls(TLS_KEYFILE,TLS_PASSWORD);
	srv->tls_client = client_tls("certs");
	srv->ec = NULL;
	srv->fc = NULL;
	srv->sc = NULL;
	srv->time = time(NULL);
	srv->numevents = 2;
	monitor_socket(srv->http_sock);
	monitor_socket(srv->tls_sock);
	srv->done = 0;
	general_signal_handlers();
	socket_signal_handlers();
	init_strings();
}

int
external_port(int fd)
{
	return fd == srv->http_sock || fd == srv->tls_sock;
}

void
set_SockReqResp(Socket sc, Request rq, Response rsp)
{
	Resp = NULL;	
	Req = NULL;
	Sock = NULL;
	if (rsp) {
		Resp = rsp;
		Req = Resp->req;
		Sock = Resp->sc;
	} else if (rq) {
		Req = rq;
		Sock = Req->sc;
	} else if (sc) {
		Sock = sc;
	}
}

void
run()
{
	File fc;
	Event ec = poll_events();
	for (srv->ec = NULL; ec; ec = ec->next) {
		set_SockReqResp(NULL,NULL,NULL);
		if (!ec->fd) continue;
		if (external_port(ec->fd)) {
			incoming(ec->fd);
			continue;
		}
		switch (ec->type) {
		case READ:
			set_SockReqResp(NULL,(Request)ec->data,NULL);
			closed_socket(Sock,"Read failed") ?
				disconnect():
				read_request();
			break;
		case WRITE:
			set_SockReqResp(NULL,NULL,(Response)ec->data);
			closed_socket(Sock,"Write failed") ?
				disconnect() :
				write_response();
			break;
		case REQ:
			set_SockReqResp(NULL,(Request)ec->data,NULL);
			closed_socket(Sock,"Request failed") ?
				disconnect() :
				write_request();
			break;	
		case RESP:
			set_SockReqResp(NULL,NULL,(Response)ec->data);
			closed_socket(Sock,"Response failed") ?
				disconnect() :
				read_response();
			break;	
		case NODE:
			fc = (File)ec->data;
			unload(ec->fd,copy(fc->name,0));
			break;
		default:
			debug("UNKNOWN EVENT");
		}
	}
	if (srv->done) {
		stop();
		exit(0);
	}
}

void
stop()
{
	File fc;
	Socket sc;
	close(srv->http_sock);
	close(srv->tls_sock);
	srv->sock = 0;
	close(srv->kq);
	srv->kq = 0;
	srv->ec = NULL;
	srv->numevents = 0;
	for (fc = srv->fc; fc; fc = close_file(fc,copy(fc->name,0)));
	srv->fc = NULL;
	for (sc = srv->sc; sc; sc = close_socket(sc));
	srv->sc = NULL;	
	srv->kq = 0;
	srv->done = 0;
}
