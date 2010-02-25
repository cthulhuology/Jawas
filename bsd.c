// bsd.c
// 
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "events.h"
#include "server.h"

#define NODE_FLAGS NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE

#ifdef BITS64
struct kevent64_s cl[MAX_EVENTS];
struct kevent64_s el[MAX_EVENTS];
#else
struct kevent cl[MAX_EVENTS];
struct kevent el[MAX_EVENTS];
#endif

struct timespec ts = { 0, 100000 };

Event
poll_events()
{
	int n;
	Event retval = NULL;
	Event e = server.event;
	server.event = NULL;
	memset(cl,0,sizeof(cl));
	memset(el,0,sizeof(el));
	for (n = 0; e; ++n) {
		debug("Queue Event: %p fd: %i type: %i next: %p",e,e->fd,e->type,e->next);
		cl[n].ident = e->fd;
		cl[n].filter = (e->type == READ || e->type == RESP ? EVFILT_READ :
				e->type == WRITE || e->type == REQ ? EVFILT_WRITE :
				e->type == NODE ? EVFILT_VNODE : 0);
		cl[n].flags = EV_ADD | (e->flag == ONESHOT ? EV_ONESHOT : 0);;
		cl[n].fflags = (e->type == NODE ? NODE_FLAGS : 0);
		cl[n].data = 0;
		cl[n].udata = (reg)e;
		e = e->next;
	}
#ifdef BITS64
	n = kevent64(server.kq,cl,n,el,server.numevents,0, &ts);
#else
	n = kevent(server.kq,cl,n,el,server.numevents,&ts);
#endif
	if (n < 0) goto done;
	while (n--)  {
		e = (Event)el[n].udata;
		e->next = retval;
		retval = e;
	}
done:
	return retval;
}
