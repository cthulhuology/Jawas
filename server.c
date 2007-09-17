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
#include "server.h"
#include "uri.h"
#include "methods.h"
#include "jws.h"

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
	old_scratch_pad = gscratch;
	set_scratch(Sock->scratch);
}

File
load(str filename)
{
	File retval;
	if (!filename) return NULL;
	server_scratch();	
	debug("Opening %s\n",filename);
	retval = query_cache(&srv->fc,filename);
	if (retval) {
		old_scratch();
		return retval;
	}
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
	server_scratch();
	debug("Unloading %s\n",filename);
	srv->fc = close_file(srv->fc,filename);
	old_scratch();
}

void
incoming(int fd)
{
	Request req;
	debug("INCOMING START");
	Scratch scratch = new_scratch(NULL);
	set_scratch(scratch);
	srv->sc = accept_socket(srv->sc,fd,(srv->http_sock == fd ? NULL : srv->tls));
	req = open_request(srv->sc);
	add_read_socket(srv->sc->fd,req);
	notice("%i.%i.%i.%i:%i connected\n",
		(0xff & srv->sc->peer),
		(0xff00 & srv->sc->peer) >> 8,
		(0xff0000 & srv->sc->peer) >> 16,
		(0xff000000 & srv->sc->peer) >> 24,
		srv->sc->port);
	debug("INCOMING DONE");
}

void
disconnect()
{
	Socket tmp,last;
	last = NULL;
	server_scratch();
	debug("DISCONNECT START");
	for (tmp = srv->sc; tmp; tmp = tmp->next) {
		debug("disconnect  %p vs %p ", tmp,Sock);
		if (tmp == Sock)  {
			notice("%i.%i.%i.%i:%i disconnected\n",
				(0xff & Sock->peer),
				(0xff00 & Sock->peer) >> 8,
				(0xff0000 & Sock->peer) >> 16,
				(0xff000000 & Sock->peer) >> 24,
		Sock->port);
			if (last) last->next = close_socket(Sock);
			else srv->sc = close_socket(Sock);
		}
		last = tmp;
	}
	debug("DISCONNECT DONE");
}

void
request()
{
	char* filename;
	File fc;
	client_scratch();
	debug("REQUEST START");
	if (!read_request(Req)) {
		old_scratch();
		error("Failed to read request\n");
		close_request(Req);
		disconnect();
		debug("REQUEST DONE");
		return;
	}
	if (Req->done) {
		Sock->buf = NULL;
		Resp = process_request(Req);
		parse_path(Req);	
		if (! parse_host(Req)) 
			Resp->status = error_handler(400);
		else 
			Resp->status = dispatch_method(parse_method(Req));
		add_write_socket(Sock->fd,Resp);
	}
	debug("REQUEST DONE");
	old_scratch();
}

void
respond()
{
	debug("RESPOND START");
	client_scratch();
	connection(Resp->headers,"close");
	transfer_encoding(Resp->headers,"identity");
	if (send_response(Resp)) {
		old_scratch();
		add_write_socket(Sock->fd,Resp);
		debug("RESPOND CONTINUE");
		return;
	}
	old_scratch();
	close_response(Resp);
	disconnect();
	debug("RESPOND DONE");
}

void
serve(int port, int tls_port)
{
	Scratch scratch = new_scratch(NULL);
	srv = (Server)alloc_scratch(scratch,sizeof(struct server_struct));
	srv->scratch = scratch;
	server_scratch();
	set_cwd();
	open_log();
#ifdef LINUX
	srv->inote = inotify_init();
	srv->kq = epoll_create(1024);
#else
	srv->kq = kqueue();
#endif
	srv->http_sock = open_socket(port);
	srv->tls_sock = open_socket(tls_port);
	srv->tls = init_tls(TLS_KEYFILE,TLS_PASSWORD);
	srv->ec = NULL;
	srv->fc = NULL;
	srv->sc = NULL;
	srv->numevents = 2;
	monitor_socket(srv->http_sock);
	monitor_socket(srv->tls_sock);
	srv->done = 0;
	socket_signal_handlers();
}

void
run()
{
	File fc;
	Event ec;
	ec = poll_events(srv->ec,srv->numevents);
	srv->numevents = 2;
	for (srv->ec = NULL; ec; ec = ec->next) {
		server_scratch();
		Req = NULL;
		Resp = NULL;
		Sock = NULL;
		switch (ec->type) {
		case READ:
			if (ec->flag == SEOF) break;
			if (ec->fd == srv->http_sock
			|| ec->fd == srv->tls_sock) {
				incoming(ec->fd);
				break;
			}
#ifdef LINUX
			if (ec->fd == srv->inote) {
				ec = file_monitor(ec);
				break;
			}
#endif
			Req = (Request)ec->data;
			Sock = Req->sc;
			request();
			break;
		case WRITE:
			Resp = (Response)ec->data;
			Sock = Resp->sc;
			Req = Resp->req;
			respond();
			break;
		case NODE:
			fc = (File)ec->data;
			unload(ec->fd,char_str(fc->name,0));
			break;
		}
	}
}

void
stop()
{
	File fc;
	Event ec;
	Socket sc;
	server_scratch();
	close(srv->http_sock);
	close(srv->tls_sock);
	srv->sock = 0;
	close(srv->kq);
	srv->kq = 0;
	free_events();
	srv->ec = NULL;
	srv->numevents = 0;
	for (fc = srv->fc; fc; fc = close_file(fc,char_str(fc->name,0)));
	srv->fc = NULL;
	for (sc = srv->sc; sc; sc = close_socket(sc));
	srv->sc = NULL;	
	srv->kq = 0;
	srv->done = 0;
}


