// client.h
//
// © 2010 David J. Goehrig
// 

#ifndef __HAVE_CLIENT_H__
#define __HAVE_CLIENT_H__

#include "events.h"
#include "sockets.h"
#include "requests.h"
#include "responses.h"

typedef struct client_struct Client;
struct client_struct {
	reg kq;
	reg alarm;
	enum event_types event;
	Socket socket;
	Request request;
	Response response;
};
extern Client client;

void handle(reg fd);

#endif
