// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "memory.h"
#include "log.h"
#include "defines.h"
#include "server.h"
#include "events.h"

extern int region_index;
extern struct region_list_struct region_list[MAX_REGIONS];

int event_index = 0;

Event
current_event()
{
	if (event_index < MAX_EVENTS)
		return region_list[region_index].address->events[event_index++] = (Event)reserve(sizeof(struct event_struct));
	return NULL;
}

Event
queue_event(Event ec, int fd, enum event_types type, enum event_flags flag, void* udata)
{
	Event retval = udata ? current_event() : (Event) system_reserve(sizeof(struct event_struct));
	retval->next = ec;
	retval->fd = fd;
	retval->type = type;	
	retval->flag = flag;
	retval->data = udata;
	debug("Event %p fd:%i type:%i data:%p next:%p",retval,retval->fd,retval->type,retval->data,retval->next);
	return retval;
}

void 
monitor_socket(int f) 
{
	srv->ec = queue_event(srv->ec,f, READ, NONE, NULL); 
	srv->numevents++;
}

void
add_read_socket(int f, Request r)
{
	srv->ec = queue_event(srv->ec,f, READ, ONESHOT, r); 
	srv->numevents++;
}

void
add_write_socket(int f, Response r) 
{
	srv->ec = queue_event(srv->ec,f, WRITE, ONESHOT, r); 
	srv->numevents++; 
}

void
add_req_socket(int f, Request r) 
{
	srv->ec = queue_event(srv->ec,f, REQ, ONESHOT, r); 
	srv->numevents++;
}

void
add_resp_socket(int f, Response r) 
{
	srv->ec = queue_event(srv->ec,f,RESP, ONESHOT, r); 
	srv->numevents++;
}

void
add_file_monitor(int f, File r)
{
	srv->ec = queue_event(srv->ec,f, NODE, ONESHOT, r);
	srv->numevents++;
}

