// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "alloc.h"
#include "log.h"
#include "defines.h"
#include "server.h"
#include "events.h"
Scratch escratch = NULL;

EventData
new_event_data(enum event_types type, void* udata) 
{
	EventData retval = (EventData)salloc(sizeof(struct event_data_wrapper));
	retval->type = type;
	switch(type) {
		case READ:
			retval->value.req = (Request)udata;
		case WRITE:
			retval->value.resp = (Response)udata;
		case REQ:
			retval->value.req = (Request)udata;
		case RESP:
			retval->value.resp = (Response)udata;
		case NODE:
			retval->value.file = (File)udata;
	}
	return retval;
}

Event
queue_event(Event ec, int fd, enum event_types type, enum event_flags flag, void* udata)
{
//	debug("Queuing %i",type);
	if (! escratch) escratch = new_scratch(NULL);
	Event retval = (Event)alloc_scratch(escratch,sizeof(struct event_cache_struct));
	retval->fd = fd;
	retval->type = type;	
	retval->flag = flag;
	retval->data = new_event_data(type,udata);
	retval->next = ec;
	retval->pos = (ec ? ec->pos + 1 : 1);
	return retval;
}

void 
monitor_socket(int f) 
{
	srv->ec = queue_event(srv->ec,f, READ, NONE, NULL); 
	srv->numevents++;
}

void
add_read_socket(int f, void* r)
{
	srv->ec = queue_event(srv->ec,f, READ, ONESHOT, r); 
	srv->numevents++;
}

void
add_write_socket(int f, void* r) 
{
	srv->ec = queue_event(srv->ec,f, WRITE, ONESHOT, r); 
	srv->numevents++; 
}

void
add_req_socket(int f, void* r) 
{
	srv->ec = queue_event(srv->ec,f, REQ, ONESHOT, r); 
	srv->numevents++;
}

void
add_resp_socket(int f, void* r) 
{
	srv->ec = queue_event(srv->ec,f,RESP, ONESHOT, r); 
	srv->numevents++;
}

void*
event_data(EventData ed)
{
	switch(ed->type) {
		case READ:
			return ed->value.req;
		case WRITE:
			return ed->value.resp;
		case REQ:
			return ed->value.req;
		case RESP:
			return ed->value.resp;
		case NODE:
			return ed->value.file;
	}
	return NULL;
}

enum event_types
event_type(EventData ed)
{
//	debug("EventData <%p> type %i",ed,ed->type);
	return ed->type;
}


