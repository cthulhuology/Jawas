// Server.h
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __SERVER_H__
#define __SERVER_H__

#include "events.h"
#include "files.h"
#include "sockets.h"

typedef struct server_struct* Server;
struct server_struct {
	int kq;
	int sock;
	Event ec;
	File fc;
	Socket sc;
	int numevents;
	int done;
};	

File load(Server srv, char* filename);

void incoming(Server srv);
void request(Server srv, Event ec);
void respond(Server srv, Event ec);
void unload(Server srv, int fd, char* filename);

Server serve(char* host, short port);
Server run(Server srv);
void stop(Server srv);


#endif
