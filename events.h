// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reservered
//

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "memory.h"
#include "sockets.h"
#include "requests.h"
#include "responses.h"

enum event_types { READ, WRITE, RESP, REQ, NODE };
enum event_flags { NONE, ONESHOT, SEOF };

struct event_struct {
	Event next;
	Socket socket;
	Request request;
	Response response;
	File file;
	uint64_t fd;
	enum event_types type;
	enum event_flags flag;
};

extern Event events;

Event queue_event(Event ec, uint64_t fd, enum event_types type, enum event_flags flag, Event e);
Event poll_events();
Event file_monitor(Event ec);

void monitor_socket(uint64_t f);
void add_read_socket(uint64_t f, Request r);
void add_write_socket(uint64_t f, Response r);
void add_req_socket(uint64_t f, Request r);
void add_resp_socket(uint64_t f, Response r);
void add_file_monitor(uint64_t f, File r);

void dump_event(Event e);

#endif
