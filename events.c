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

Event events;

Event
current_event()
{
	if (region_list[region_index].event_index >= MAX_EVENTS)
		region_list[region_index].event_index = 0;
	return region_list[region_index].address->events[region_list[region_index].event_index++] = (Event)reserve(sizeof(struct event_struct));
}

Event
queue_event(Event ec, uint64_t fd, enum event_types type, enum event_flags flag, Event e)
{
	Event retval = e ? e : (Event) system_reserve(sizeof(struct event_struct));
	retval->next = ec;
	retval->fd = fd;
	retval->type = type;
	retval->flag = flag;
	retval->socket = e ? e->socket : NULL;
	retval->request = e ? e->request : NULL;
	retval->response = e ? e->response : NULL;
	retval->file = e ? e->file : NULL;
	debug("Event %p fd:%i type:%i sock:%p req:%p resp:%p file:%p next:%p",retval,retval->fd,retval->type,retval->socket,retval->request,retval->response,retval->file,retval->next);
	return retval;
}

void 
monitor_socket(uint64_t f) 
{
	server.event = queue_event(server.event,f, READ, NONE, NULL); 
	server.event->socket = NULL;
	server.event->request = NULL;
	server.event->response = NULL;
	server.event->file = NULL;
	server.numevents++;
}

void
add_read_socket(uint64_t f, Request r)
{
	Event e = current_event();
	e->socket =  r ? r->socket : NULL;
	e->request = r;
	e->response = NULL;
	e->file = NULL;
	server.event = queue_event(server.event,f, READ, ONESHOT, e); 
	server.numevents++;
}

void
add_write_socket(uint64_t f, Response r) 
{
	Event e = current_event();
	e->socket = r ? r->socket : NULL;
	e->request = r ? r->request : NULL;
	e->response = r;
	e->file = NULL;
	server.event = queue_event(server.event,f, WRITE, ONESHOT, e); 
	server.numevents++; 
}

void
add_req_socket(uint64_t f, Request r) 
{
	Event e = current_event();
	e->socket =  r ? r->socket : NULL;
	e->request = r;
	e->response = NULL;
	e->file = NULL;
	server.event = queue_event(server.event,f, REQ, ONESHOT, e); 
	server.numevents++;
}

void
add_resp_socket(uint64_t f, Response r) 
{
	Event e = current_event();
	e->socket = r ? r->socket : NULL;
	e->request = r ? r->request : NULL;
	e->response = r;
	e->file = NULL;
	server.event = queue_event(server.event,f,RESP, ONESHOT, e); 
	server.numevents++;
}

void
add_file_monitor(uint64_t f, File r)
{
	Event e = current_event();
	e->socket = NULL;
	e->request = NULL;
	e->response = NULL;
	e->file = r;
	server.event = queue_event(server.event,f, NODE, ONESHOT, e);
	server.numevents++;
}
