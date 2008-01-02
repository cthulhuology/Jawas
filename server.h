// Server.h
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __SERVER_H__
#define __SERVER_H__

#include "alloc.h"
#include "str.h"
#include "events.h"
#include "files.h"
#include "tls.h"
#include "sockets.h"
#include "timers.h"

typedef struct server_struct* Server;
struct server_struct {
	int kq;
	int http_sock;
	int tls_sock;
	int alarm;
	Event ec;
	File fc;
	Socket sc;
	TLSInfo tls;
	Scratch scratch;
	Timers timers;
	time_t time;
	int numevents;
	int done;
	RequestInfo ri;
	Usage usage;
	Event event;	// current event;
	Socket sock;	// current socket;
	Request req;	// current request
	Response resp;	// current response
};

extern Server srv;

void serve(int port, int tls_port);
void run();
void stop();

File load(str filename);
void unload(int fd, str filename);

void server_scratch();
void client_scratch();

void set_SockReqResp(Socket sc, Request rq, Response rsp);

void server_scratch();
void old_scratch();

#endif
