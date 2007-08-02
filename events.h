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

void monitor_socket(int fd);
void add_read_socket(int fd, Request req);
void add_write_socket(int fd, Response resp);
void add_file_monitor(int fd, char* filename);

#define NODE_FLAGS NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE

#endif
