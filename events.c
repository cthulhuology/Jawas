// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "defines.h"
#include "include.h"
#include "memory.h"
#include "log.h"
#include "defines.h"
#include "client.h"
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
queue_event(Event ec, reg fd, enum event_types type, enum event_flags flag, Event e)
{
	Event retval = e ? e : (Event) system_reserve(sizeof(struct event_struct));
	retval->next = ec;
	retval->fd = fd;
	retval->type = type;
	retval->flag = flag;
	debug("Event %p fd:%i type:%i next:%p",retval,retval->fd,retval->type,retval->next);
	return retval;
}

void 
monitor_socket(reg f) 
{
	server.event = queue_event(server.event,f, READ, NONE, NULL); 
}

void
add_read_socket(reg f)
{
	client.event = queue_event(client.event,f, READ, ONESHOT, current_event()); 
}

void
add_write_socket(reg f)
{
	client.event = queue_event(client.event,f, WRITE, ONESHOT,current_event()); 
}

void
add_req_socket(reg f)
{
	client.event = queue_event(client.event,f, REQ, ONESHOT, current_event()); 
}

void
add_resp_socket(reg f)
{
	client.event = queue_event(client.event,f,RESP, ONESHOT, current_event()); 
}

void
add_file_monitor(reg f)
{
	server.event = queue_event(server.event,f, NODE, ONESHOT, current_event());
}
