// Server.h
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __SERVER_H__
#define __SERVER_H__

#include "alloc.h"
#include "events.h"
#include "files.h"
#include "tls.h"
#include "sockets.h"

typedef struct server_struct* Server;
struct server_struct {
	int kq;
	int sock;
	int tls_sock;
	Event ec;
	File fc;
	Socket sc;
	TLSInfo tls;
	Scratch scratch;
	int numevents;
	int done;
};

File load(Server srv, char* filename);

void incoming(Server srv, int fd);
void request(Server srv, Event ec);
void respond(Server srv, Event ec);
void unload(Server srv, int fd, char* filename);

Server serve(int port, int tls_port);
Server run(Server srv);
void stop(Server srv);


#endif
