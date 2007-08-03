// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reservered
//

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <sys/types.h>
#include <sys/event.h>
#include "requests.h"
#include "responses.h"

typedef struct event_cache_struct* Event;
struct event_cache_struct {
	struct kevent event;
	Event next;
	int pos;
};

Event poll_events(Event ec, int kq, int numevents);
void free_events();

Event queue_event(Event ec, int id, short filter, u_short flags, u_int fflags, intptr_t data, void* udata);

#define NODE_FLAGS NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE

#define monitor_socket(f) \
	srv->ec = queue_event(srv->ec,f, EVFILT_READ, EV_ADD, 0, 0, NULL); \
	srv->numevents++;

#define add_read_socket(f,r) \
	srv->ec = queue_event(srv->ec,f, EVFILT_READ, EV_ADD|EV_ONESHOT, 0, 0, r); \
	srv->numevents++;

#define add_write_socket(f,r) \
	srv->ec = queue_event(srv->ec,f, EVFILT_WRITE, EV_ADD|EV_ONESHOT, 0, 0, r); \
	srv->numevents++; 

#define add_file_monitor(f,r) \
	srv->ec = queue_event(srv->ec,f, EVFILT_VNODE, EV_ADD|EV_ONESHOT, NODE_FLAGS, 0,r);\
	srv->numevents++;

#endif
