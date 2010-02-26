// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reservered
//

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "defines.h"
#include "memory.h"
#include "sockets.h"
#include "requests.h"
#include "responses.h"

enum event_types { READ, WRITE, RESP, REQ, NODE };
enum event_flags { NONE, ONESHOT, SEOF };

struct event_struct {
	Event next;
	reg fd;
	enum event_types type;
	enum event_flags flag;
};

extern Event events;

Event poll_events();
Event file_monitor(Event ec);

void monitor_socket(reg f);
void add_read_socket(reg f);
void add_write_socket(reg f);
void add_req_socket(reg f);
void add_resp_socket(reg f);
void add_file_monitor(reg f);

void dump_event(Event e);

#endif
