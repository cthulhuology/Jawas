// jawas server.c
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "defines.h"
#include "headers.h"
#include "status.h"
#include "events.h"
#include "files.h"
#include "sockets.h"
#include "server.h"
#include "uri.h"
#include "js.h"

File
load(Server srv, char* filename)
{
	File retval;
	fprintf(stderr,"Opening %s\n",filename);
	retval = query_cache(&srv->fc,filename);
	if (retval) return retval;
	retval = open_file(srv->fc,filename);
	if (!retval) {
		fprintf(stderr,"Failed to open %s\n",filename);
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
	fprintf(stderr,"Unloading %s\n",filename);
	srv->fc = close_file(srv->fc,filename);
}

void
incoming(Server srv)
{
	Request req;
	srv->sc = accept_socket(srv->sc,srv->sock);
	req = open_request(srv->sc);
	srv->ec = add_read_socket(srv->ec, srv->sc->fd,req);
	srv->numevents++;
	fprintf(stderr,"[Jawas] Connected %d.%d.%d.%d:%d\n",
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
	fprintf(stderr,"Doing read for Request %p\n",req);
	if (!read_request(req)) {
		fprintf(stderr,"Failed to read request\n");
		disconnect(srv,req);
		close_request(req);
		return;
	}
	dump_headers(req->headers);
	
	if (req->done) {
		fprintf(stderr,"Request done\n");
		req->sc->buf = NULL;
		resp = process_request(req);
		filename = request_path(resp->req);
		fprintf(stderr,"Got filename %s\n",filename);
		fc = load(srv,filename);
		if (! fc) {
			resp->status = 404;
		} else { 
			jws_handler(srv,fc,resp);
		}
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
	fprintf(stderr,"Writing response\n");
	send_response(resp);
	disconnect(srv,resp->req);
	close_response(resp);
	dump_cache_info();
}

Server
serve(char* host, short port)
{
	Server srv = (Server)malloc(sizeof(struct server_struct));
	srv->kq = kqueue();
	if (0 > srv->kq) return NULL;
	if (0 > (srv->sock = open_socket(port))) return NULL;
	srv->ec = NULL;
	srv->fc = NULL;
	srv->sc = NULL;
	srv->numevents = 1;
	srv->ec = monitor_socket(srv->ec,srv->sock);
	fprintf(stderr,"Have event queue: %p\n",srv->ec);
	srv->done = 0;
	socket_signal_handlers();
	return srv;
}

Server
run(Server srv)
{
	Event ec;

	ec = poll_events(srv->ec,srv->kq,srv->numevents);
	srv->numevents = 1;
	for (srv->ec = NULL; ec; ec = ec->next) {
		fprintf(stderr,"Processing socket %d\n",ec->event.ident);
		switch (ec->event.filter) {
		case EVFILT_READ:
			if (ec->event.flags == EV_EOF) break;
			(ec->event.ident == srv->sock)  ?
				incoming(srv) :
				request(srv,ec);
			break;
		case EVFILT_WRITE:
			respond(srv,ec);
			break;
		case EVFILT_VNODE:
			// if (!ec->event.udata) break;
			fprintf(stderr,"VNODE on %s\n",ec->event.udata);
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

int
main(int argc, char** argv)
{
	int child = 0;
	int child_status = 0;
	int detach = 0;

	if (argc > 1 && !strncmp(argv[1],"-d",2)) {
		detach = 1;	
	}
	char* port = (argc > (1 + detach) ? argv[1 + detach] : SERVER_PORT);;
	char* host = (argc > (2 + detach) ? argv[2 + detach] : NULL);

	if (argc > 3) {
		fprintf(stderr,"Usage %s [-d] [port] [host]\n", argv[0]);
		exit(1);
	}

restart:
	if (detach) {
		child = fork();
	}
	if (child == 0) {
		Server srv = serve(host,atoi(port));
		if (!srv) return 1;
		while (! srv->done) srv = run(srv);	
		stop(srv);
	} else {
		waitpid(child,&child_status,0);
		goto restart;
	}
	return 0;
}
