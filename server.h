// Server.h
// Copyright (C) 2007 David J. Goehrig
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
	int alarm;
	time_t time;
	TLSInfo tls;
	TLSInfo tls_client;
	int done;
	int numevents;
	Event event;
	Socket socket;
	Request request;
	Response response;
	File file;
	File files[MAX_FILES];
	int file_index;
};

extern Server server;

void serve(int port, int tls_port);
void run();
void stop();

#endif
