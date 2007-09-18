// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reservered
//

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "requests.h"
#include "responses.h"

enum event_types { READ, WRITE, NODE };
enum event_flags { NONE, ONESHOT, SEOF };

typedef struct event_cache_struct* Event;
struct event_cache_struct {
	Event next;
	int pos;
	enum event_types type;
	enum event_flags flag;
	int fd;
	void* data;
};

Event poll_events(Event ec, int numevents);
void free_events();

Event queue_event(Event ec, int fd, enum event_types type, enum event_flags flag, void* udata);

Event file_monitor(Event ec);

#ifdef LINUX
#define NODE_FLAGS DN_MODIFY | DN_CREATE | DN_DELETE | DN_RENAME | DN_ATTRIB
#else
#define NODE_FLAGS NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE
#endif

#define monitor_socket(f) \
	srv->ec = queue_event(srv->ec,f, READ, NONE, NULL); \
	srv->numevents++;

#define add_read_socket(f,r) \
	srv->ec = queue_event(srv->ec,f, READ, ONESHOT, r); \
	srv->numevents++;

#define add_write_socket(f,r) \
	srv->ec = queue_event(srv->ec,f, WRITE, ONESHOT, r); \
	srv->numevents++; 

#define add_file_monitor(f,r) \
	srv->ec = queue_event(srv->ec,f, NODE, ONESHOT, r);\
	srv->numevents++;

#endif
