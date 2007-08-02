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

typedef struct server_struct* Server;
struct server_struct {
	int kq;
	int http_sock;
	int tls_sock;
	Event ec;
	File fc;
	Socket sc;
	TLSInfo tls;
	Scratch scratch;
	int numevents;
	int done;
	Event event;	// current event;
	Socket sock;	// current socket;
	Request req;	// current request
	Response resp;	// current response
};

extern Server srv;

File load(str filename);
void unload(int fd, str filename);

void serve(int port, int tls_port);
void run();
void incoming(int fd);
void request();
void respond();
void stop();

void server_scratch();
void client_scratch();

#endif
