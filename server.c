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

File
load(Server srv, str filename)
{
	File retval;
	Scratch old = gscratch;
	set_scratch(srv->scratch);
	if (!filename) return NULL;
	debug("Opening %s\n",filename);
	retval = query_cache(&srv->fc,filename);
	if (retval) return retval;
	retval = open_file(srv->fc,filename);
	if (!retval) {
		error("Failed to open %s\n",filename);
		set_scratch(old);
		return NULL;
	}
	srv->fc = retval;
	srv->ec = add_file_monitor(srv->ec,srv->fc->fd,srv->fc->name);
	srv->numevents++;
	set_scratch(old);
	return retval;
}

void
unload(Server srv, int fd, str filename)
{
	debug("Unloading %s\n",filename);
	srv->fc = close_file(srv->fc,filename);
}

void
incoming(Server srv, int fd)
{
	Request req;
	debug("INCOMING START");
	Scratch scratch = new_scratch(NULL);
	set_scratch(scratch);
	srv->sc = accept_socket(srv->sc,fd,(srv->sock == fd ? NULL : srv->tls));
	req = open_request(srv->sc);
	set_scratch(srv->scratch);
	srv->ec = add_read_socket(srv->ec, srv->sc->fd,req);
	srv->numevents++;
	notice("%i.%i.%i.%i:%i connected\n",
		(0xff & srv->sc->peer),
		(0xff00 & srv->sc->peer) >> 8,
		(0xff0000 & srv->sc->peer) >> 16,
		(0xff000000 & srv->sc->peer) >> 24,
		srv->sc->port);
	debug("INCOMING DONE");
}

void
disconnect(Server srv, Socket sc)
{
	Socket tmp,last;
	last = NULL;
	debug("DISCONNECT START");
	for (tmp = srv->sc; tmp; tmp = tmp->next) {
		debug("disconnect  %p vs %p ", tmp,sc);
		if (tmp == sc)  {
			notice("%i.%i.%i.%i:%i disconnected\n",
				(0xff & sc->peer),
				(0xff00 & sc->peer) >> 8,
				(0xff0000 & sc->peer) >> 16,
				(0xff000000 & sc->peer) >> 24,
		srv->sc->port);
			if (last) last->next = close_socket(sc);
			else srv->sc = close_socket(sc);
		}
		last = sc;
	}
	debug("DISCONNECT DONE");
}

void
request(Server srv, Event ec)
{
	char* filename;
	File fc;
	Response resp;
	debug("REQUEST START");
	Request req = (Request)ec->event.udata;
	Socket sc = req->sc;;
	set_scratch(sc->scratch);
	if (!read_request(req)) {
		error("Failed to read request\n");
		close_request(req);
		disconnect(srv,sc);
		set_scratch(srv->scratch);
		debug("REQUEST DONE");
		return;
	}
	if (req->done) {
		sc->buf = NULL;
		resp = process_request(req);
		parse_path(resp->req);	
		if (! parse_host(resp->req)) 
			resp->status = error_handler(srv,400,resp);
		else 
			resp->status = dispatch_method(srv,parse_method(req),resp);
		srv->ec = add_write_socket(srv->ec,sc->fd,resp);
		srv->numevents++;
	}
	set_scratch(srv->scratch);
	debug("REQUEST DONE");
}

void
respond(Server srv, Event ec)
{
	Request req;
	debug("RESPOND START");
	Response resp = (Response)ec->event.udata;
	Socket sc = resp->sc;
	set_scratch(sc->scratch);
	connection(resp->headers,"close");
	transfer_encoding(resp->headers,"identity");
	if (send_response(resp)) {
		srv->ec = add_write_socket(srv->ec,sc->fd,resp);
		srv->numevents++;
		set_scratch(srv->scratch);
		debug("RESPOND CONTINUE");
		return;
	}
	close_response(resp);
	disconnect(srv,sc);
	debug("RESPOND DONE");
}

Server
serve(int port, int tls_port)
{
	Scratch scratch = new_scratch(NULL);
	Server srv = (Server)alloc_scratch(scratch,sizeof(struct server_struct));
	srv->scratch = scratch;
	set_scratch(srv->scratch);
	srv->kq = kqueue();
	if (0 > srv->kq) return NULL;
	if (0 > (srv->sock = open_socket(port))) return NULL;
	if (0 > (srv->tls_sock = open_socket(tls_port))) return NULL;
	srv->tls = init_tls(TLS_KEYFILE,TLS_PASSWORD);
	srv->ec = NULL;
	srv->fc = NULL;
	srv->sc = NULL;
	srv->numevents = 2;
	srv->ec = monitor_socket(srv->ec,srv->sock);
	if (tls_port) srv->ec = monitor_socket(srv->ec,srv->tls_sock);
	srv->done = 0;
	socket_signal_handlers();
	return srv;
}

Server
run(Server srv)
{
	Event ec;

	ec = poll_events(srv->ec,srv->kq,srv->numevents);
	srv->numevents = 2;
	for (srv->ec = NULL; ec; ec = ec->next) {
		switch (ec->event.filter) {
		case EVFILT_READ:
			if (ec->event.flags == EV_EOF) break;
			(ec->event.ident == srv->sock || ec->event.ident == srv->tls_sock)  ?
				incoming(srv,ec->event.ident) :
				request(srv,ec);
			break;
		case EVFILT_WRITE:
			respond(srv,ec);
			break;
		case EVFILT_VNODE:
			unload(srv,ec->event.ident,ec->event.udata);
			break;
		}
	}
	return srv;
}

void
stop(Server srv)
{
	File fc;
	Event ec;
	Socket sc;

	close(srv->sock);
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

