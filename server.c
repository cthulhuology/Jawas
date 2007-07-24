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
#include "jws.h"

File
load(Server srv, char* filename)
{
	File retval;
	if (!filename) return NULL;
	debug("Opening %s\n",filename);
	retval = query_cache(&srv->fc,filename);
	if (retval) return retval;
	retval = open_file(srv->fc,filename);
	if (!retval) {
		error("Failed to open %s\n",filename);
		return NULL;
	}
	srv->fc = retval;
	srv->ec = add_file_monitor(srv->ec,srv->fc->fd,srv->fc->name);
	srv->numevents++;
	return retval;
}

void
unload(Server srv, int fd, char* filename)
{
	debug("Unloading %s\n",filename);
	srv->fc = close_file(srv->fc,filename);
}

void
incoming(Server srv, int fd)
{
	Request req;
	srv->sc = accept_socket(srv->sc,fd,(srv->sock == fd ? NULL : srv->tls));
	req = open_request(srv->sc);
	srv->ec = add_read_socket(srv->ec, srv->sc->fd,req);
	srv->numevents++;
	notice("[Jawas] Connected %i.%i.%i.%i:%i\n",
		(0xff & srv->sc->peer),
		(0xff00 & srv->sc->peer) >> 8,
		(0xff0000 & srv->sc->peer) >> 16,
		(0xff000000 & srv->sc->peer) >> 24,
		srv->sc->port);
}

void
disconnect(Server srv, Request req)
{
	Socket sc, last;
	last = NULL;
	for (sc = srv->sc; sc; sc = sc->next) {
		if (sc == req->sc)  {
			if (last) {
				last->next = close_socket(sc);
			} else {
				srv->sc = close_socket(sc);
			}
		}
		last = sc;
	}
}

void
request(Server srv, Event ec)
{
	char* filename;
	File fc;
	Response resp;
	Request req = (Request)ec->event.udata;
	if (!read_request(req)) {
		error("Failed to read request\n");
		disconnect(srv,req);
		close_request(req);
		return;
	}
	if (req->done) {
		req->sc->buf = NULL;
		resp = process_request(req);
		filename = request_path(resp->req);
		fc = is_directory(filename) ? 
			load(srv,get_index(filename)) :
			load(srv,filename);
		resp->status =  mimetype_handler(srv,fc,resp);
		srv->ec = add_write_socket(srv->ec,req->sc->fd,resp);
		srv->numevents++;
	}
}

void
respond(Server srv, Event ec)
{
	Request req;
	Response resp = (Response)ec->event.udata;
	Socket sc = resp->sc;
	send_response(resp);
	disconnect(srv,resp->req);
	close_response(resp);
	dump_cache_info();
}

Server
serve(int port, int tls_port)
{
	Server srv = (Server)malloc(sizeof(struct server_struct));
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
	for (ec = srv->ec; ec; ec = free_event(ec));
	srv->ec = NULL;
	srv->numevents = 0;
	for (fc = srv->fc; fc; fc = close_file(fc,fc->name));
	srv->fc = NULL;
	for (sc = srv->sc; sc; sc = close_socket(sc));
	srv->sc = NULL;	
	srv->kq = 0;
	srv->done = 0;
}

