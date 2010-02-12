// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reservered
//

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "memory.h"
#include "requests.h"
#include "responses.h"

enum event_types { READ, WRITE, RESP, REQ, NODE };
enum event_flags { NONE, ONESHOT, SEOF };

struct event_struct {
	Event next;
	int fd;
	enum event_types type;
	enum event_flags flag;
	void* data;
};

Event queue_event(Event ec, int fd, enum event_types type, enum event_flags flag, void* udata);
Event poll_events();
Event file_monitor(Event ec);

void monitor_socket(int f);
void add_read_socket(int f, Request r);
void add_write_socket(int f, Response r);
void add_req_socket(int f, Request r);
void add_resp_socket(int f, Response r);
void add_file_monitor(int f, File r);

#endif
