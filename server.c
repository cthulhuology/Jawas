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
#include "strings.h"
#include "jws.h"
#include "sms.h"

Server srv = NULL;
Scratch old_scratch_pad;

void
server_scratch()
{
	old_scratch_pad = gscratch;
	set_scratch(srv->scratch);
}

void
old_scratch()
{
	set_scratch(old_scratch_pad);
	old_scratch_pad = srv->scratch;
}

void
client_scratch()
{
	if (Sock) {
		old_scratch_pad = gscratch;
		set_scratch(Sock->scratch);
	}
}

File
load(str filename)
{
	File retval = NULL;
	if (!filename) return NULL;
	server_scratch();	
	retval = query_cache(&srv->fc,filename);
#ifdef LINUX
	if (retval) 
		unload(retval->fd,filename);
#else
	if (retval) {
		old_scratch();
		return retval;
	}
#endif
	retval = open_file(srv->fc,filename);
	if (!retval) {
		error("Failed to open %s\n",filename);
		old_scratch();
		return NULL;
	}
	srv->fc = retval;
	add_file_monitor(srv->fc->fd,srv->fc);
	old_scratch();
	return retval;
}

void
unload(int fd, str filename)
{
	Timers t;
	server_scratch();
	for (t = srv->timers; t; t = t->next) {
		char* fname = dump(filename);
		if (!strcmp(fname,t->script->name)) {
			free(fname);
			reopen_file(t->script);
			old_scratch();
			return;
		}
		free(fname);
	}
	srv->fc = close_file(srv->fc,filename);
	old_scratch();
}

void
resume(Socket sc)
{
	Request req;
	req = open_request(srv->sc);
	srv->ri = start_request(srv->ri,req);
	add_read_socket(srv->sc->fd,req);
	reset_socket(srv->sc);
	socket_notice(srv->sc,"Reset");
}

void
incoming(int fd)
{
	Scratch scratch = new_scratch(NULL);
	set_scratch(scratch);
	srv->sc = accept_socket(srv->sc,fd,(srv->http_sock == fd ? NULL : srv->tls));
	socket_notice(srv->sc,"Connected");
	resume(srv->sc);
}

void
disconnect()
{
	Socket tmp,last;
	last = NULL;
	server_scratch();
	socket_notice(Sock,"Closing");
	for (tmp = srv->sc; tmp; tmp = tmp->next) {
		if (tmp == Sock)  {
			socket_notice(Sock,"Disconnected");
			last ? 
				(last->next = close_socket(Sock)) :
				(srv->sc = close_socket(Sock));
		}
		last = tmp;
	}
}

void
read_request()
{
	client_scratch();
	if (!process_request(Req)) {
		old_scratch();
		debug("Request retries %i vs %i",Req->retries, MAX_RETRIES);
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
		str host = parse_host(Req);
		str method = parse_method(Req);
		Resp->status = host && method ?
			dispatch_method(method) :
			error_handler(400);
		if (Resp->status > 0)
			add_write_socket(Sock->fd,Resp);
		if (Resp->status == 0)
			debug("Response pending processing!");
	} else {
		add_read_socket(Sock->fd,Req);
	}
	old_scratch();
}

void
read_response()
{
	client_scratch();
	if (!process_response(Resp)) {
		old_scratch();
		error("Failed to read response\n");
		disconnect();		
		return;
	}
	if (Resp->done) {
		Response tmp = Resp;
		str cb = tmp->req->cb;
		Headers hdrs = tmp->headers;
		append_header(hdrs,Str("data"),from(tmp->contents,tmp->body,len(tmp->contents) - tmp->body));
		append_header(hdrs,Str("status"),from(tmp->contents,9,3));
		set_SockReqResp(NULL,NULL,Resp->req->resp);
		process_callback(cb,hdrs);
		connection(Resp->headers,"close");
//		transfer_encoding(Resp->headers,"chunked");
		adopt_scratch(Resp->sc->scratch,tmp->sc->scratch);
		tmp->sc->scratch = NULL;
		close_socket(tmp->sc);
		add_write_socket(Resp->sc->fd,Resp);
	} else {
		add_resp_socket(Sock->fd,Resp);
	}
	// if (Resp->done)
	// 	disconnect();
	old_scratch();
}

void
write_response()
{
	client_scratch();
	if (send_response(Resp)) {
		old_scratch();
		add_write_socket(Sock->fd,Resp);
	//	fprintf(stderr,"Continuing\n");
		return;
	}
	old_scratch();
	srv->ri = end_request(srv->ri,Resp->req);
//	resume(srv->sc);
	disconnect();
}

void
write_request()
{
	client_scratch();
	if (send_request(Req)) {
		old_scratch();
		add_req_socket(Sock->fd,Req);
		return;
	}
	old_scratch();
	add_resp_socket(Sock->fd,new_response(Req));
}

str
load_config(char* filename)
{
	File conf = open_file(srv->fc,Str("%c",filename));
	if (conf) {
		srv->fc = conf;
		return copy(conf->data,conf->st.st_size-1);		
	}
	return NULL;
}

void
serve(int port, int tls_port)
{
	Scratch scratch = new_scratch(NULL);
	srv = (Server)alloc_scratch(scratch,sizeof(struct server_struct));
	srv->scratch = scratch;
	set_scratch(NULL);
	server_scratch();
	set_cwd();
#ifdef LINUX
	srv->kq = epoll_create(1024);
#else
	srv->kq = kqueue();
#endif
	srv->alarm = 0;
	srv->http_sock = open_socket(port);
	srv->tls_sock = open_socket(tls_port);
	srv->tls = init_tls(TLS_KEYFILE,TLS_PASSWORD);
	srv->tls_client = client_tls("certs");
	srv->usage = new_usage(1);
	srv->ec = NULL;
	srv->fc = NULL;
	srv->sc = NULL;
	srv->timers = NULL;
	srv->time = time(NULL);
	srv->numevents = 2;
	monitor_socket(srv->http_sock);
	monitor_socket(srv->tls_sock);
	srv->done = 0;
	general_signal_handlers();
	socket_signal_handlers();
#ifdef LINUX
	file_signal_handlers();
#endif
	init_strings();
	init_timers();
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
	Event ec = srv->ec;
	int events = srv->numevents;
	srv->ec = NULL;
	srv->numevents = 2;
	server_scratch();
	set_SockReqResp(NULL,NULL,NULL);
	update_timers();
	ec = poll_events(ec,events);
	for (srv->ec = NULL; ec; ec = ec->next) {
		server_scratch();
		start_usage(srv->usage);
		set_SockReqResp(NULL,NULL,NULL);
		if (ec->fd == 0) continue;
		if (external_port(ec->fd)) {
			incoming(ec->fd);
			continue;
		}
		switch (ec->type) {
		case READ:
			set_SockReqResp(NULL,(Request)event_data(ec->data),NULL);
			closed_socket(Sock,"Read failed") ?
				disconnect():
				read_request();
			break;
		case WRITE:
			set_SockReqResp(NULL,NULL,(Response)event_data(ec->data));
			closed_socket(Sock,"Write failed") ?
				disconnect() :
				write_response();
			break;
		case REQ:
			set_SockReqResp(NULL,(Request)event_data(ec->data),NULL);
			closed_socket(Sock,"Request failed") ?
				disconnect() :
				write_request();
			break;	
		case RESP:
			set_SockReqResp(NULL,NULL,(Response)event_data(ec->data));
			closed_socket(Sock,"Response failed") ?
				disconnect() :
				read_response();
			break;	
		case NODE:
			fc = (File)event_data(ec->data);
			unload(ec->fd,copy(fc->name,0));
			break;
		default:
			debug("UNKNOWN EVENT");
		}
		stop_usage(srv->usage);
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
	server_scratch();
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
