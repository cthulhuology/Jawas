// Server.h
// Copyright (C) 2007,2010 David J. Goehrig
// All Rights Reserved
//

#ifndef __SERVER_H__
#define __SERVER_H__

#include "str.h"
#include "files.h"
#include "sockets.h"
#include "requests.h"
#include "responses.h"
#include "tls.h"
#include "timers.h"

typedef struct server_struct Server;
struct server_struct {
	reg kq;
	reg http_sock;
	reg tls_sock;
	time_t time;
	TLSInfo tls;
	TLSInfo tls_client;
	Event event;
	str cwd;
	File files[MAX_FILES];
	int file_index;
	int done;
};

extern Server server;

void serve(int port, int tls_port);
void stop();

#endif
