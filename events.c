// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "events.h"
#include "defines.h"

Event
queue_event(Event ec, int id, short filter, u_short flags, u_int fflags, intptr_t data, void* udata)
{
	Event retval = (Event)malloc(sizeof(struct event_cache_struct));
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

Event
free_event(Event ec)
{
	if (! ec) return NULL;
	Event retval = ec->next;
	free(ec);
	return retval;
}

Event
poll_events(Event ec, int kq, int numevents)
{
	Event retval = NULL;
	int n;
	struct timespec ts = { 0, 0 };

	struct kevent* cl = (struct kevent*)(ec ? malloc(sizeof(struct kevent)*ec->pos) : NULL);
	if (cl) memset(cl,0,sizeof(struct kevent)*ec->pos);
	struct kevent* el = (struct kevent*)malloc(sizeof(struct kevent)*numevents);
	if (el) memset(el,0,sizeof(struct kevent)*numevents);

	for (n = 0; ec; ++n) {
		memcpy(&cl[n],&ec->event,sizeof(struct kevent));
		ec = free_event(ec);
	}
	n = kevent(kq,cl,n,el,numevents, NULL); // &ts);
	retval = NULL;	
	while (n--) {
		retval = queue_event(retval,el[n].ident,el[n].filter,el[n].flags,el[n].fflags,el[n].data,el[n].udata);
	}
	if (cl) free(cl);
	if (el) free(el);
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
	fprintf(stderr,"Adding read socket %p\n",req);
	return queue_event(ec, fd, EVFILT_READ, EV_ADD|EV_ONESHOT, 0, 0, req);
}

Event
add_write_socket(Event ec, int fd, Response resp)
{
	fprintf(stderr,"Adding write socket for request %p\n",resp->req);
	return queue_event(ec, fd, EVFILT_WRITE, EV_ADD|EV_ONESHOT, 0, 0, resp);
}

Event
add_file_monitor(Event ec, int fd, char* filename)
{
	return queue_event(ec, fd, EVFILT_VNODE, EV_ADD|EV_ONESHOT, NODE_FLAGS, 0, filename);
}

