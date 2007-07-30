// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "events.h"
#include "log.h"
#include "defines.h"

Scratch escratch = NULL;

Event
queue_event(Event ec, int id, short filter, u_short flags, u_int fflags, intptr_t data, void* udata)
{
	if (escratch == NULL) escratch = new_scratch(NULL);
	Event retval = (Event)alloc_scratch(escratch,sizeof(struct event_cache_struct));
	retval->event.ident = id;
	retval->event.filter  = filter;
	retval->event.flags = flags;
	retval->event.fflags = fflags;
	retval->event.data = data;
	retval->event.udata = udata;
	retval->next = ec;
	retval->pos = (ec ? ec->pos + 1 : 1);
	return retval;
}

void
free_events()
{
	if (escratch) free_scratch(escratch);
	escratch = NULL;
}

Event
poll_events(Event ec, int kq, int numevents)
{
	int n;
	Scratch scratch = new_scratch(NULL);
	Event retval = NULL;
	struct timespec ts = { 0, 0 };
	struct kevent* cl = (struct kevent*)(ec ? alloc_scratch(scratch,sizeof(struct kevent)*ec->pos) : NULL);
	struct kevent* el = (struct kevent*)alloc_scratch(scratch,sizeof(struct kevent)*numevents);

	for (n = 0; ec; ++n) {
		memcpy(&cl[n],&ec->event,sizeof(struct kevent));
		ec = ec->next;
	}
	free_events();
	n = kevent(kq,cl,n,el,numevents, NULL);
	retval = NULL;	
	while (n--) 
		retval = queue_event(retval,el[n].ident,el[n].filter,el[n].flags,el[n].fflags,el[n].data,el[n].udata);
	free_scratch(scratch);
	return retval;
}

Event
monitor_socket(Event ec, int fd)
{
	return queue_event(ec, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
}

Event
add_read_socket(Event ec, int fd, Request req)
{
	return queue_event(ec, fd, EVFILT_READ, EV_ADD|EV_ONESHOT, 0, 0, req);
}

Event
add_write_socket(Event ec, int fd, Response resp)
{
	return queue_event(ec, fd, EVFILT_WRITE, EV_ADD|EV_ONESHOT, 0, 0, resp);
}

Event
add_file_monitor(Event ec, int fd, char* filename)
{
	return queue_event(ec, fd, EVFILT_VNODE, EV_ADD|EV_ONESHOT, NODE_FLAGS, 0, filename);
}

