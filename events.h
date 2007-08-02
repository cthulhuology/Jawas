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
Event monitor_socket(Event ec, int fd);
Event add_read_socket(Event ec, int fd, Request req);
Event add_write_socket(Event ec, int fd, Response resp);
Event add_file_monitor(Event ec, int fd, char* filename);

#define NODE_FLAGS NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE

#endif
