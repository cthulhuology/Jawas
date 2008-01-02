// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reservered
//

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "requests.h"
#include "responses.h"

enum event_types { READ, WRITE, RESP, REQ, NODE };
enum event_flags { NONE, ONESHOT, SEOF };

typedef struct event_data_wrapper* EventData;
struct event_data_wrapper {
	enum event_types type;
	union {
		Request req;
		Response resp;
		File file;
	} value;
};

typedef struct event_cache_struct* Event;
struct event_cache_struct {
	Event next;
	int pos;
	enum event_types type;
	enum event_flags flag;
	int fd;
	EventData data;
};


Event queue_event(Event ec, int fd, enum event_types type, enum event_flags flag, void* udata);
Event poll_events(Event ec, int numevents);
Event file_monitor(Event ec);

void monitor_socket(int f);
void add_read_socket(int f, void* r);
void add_write_socket(int f, void* r);
void add_req_socket(int f, void* r);
void add_resp_socket(int f, void* r);
void add_file_monitor(int f, void* r);

void* event_data(EventData ed);
EventData new_event_data(enum event_types type, void* udata);
enum event_types event_type(EventData ed);

#endif
