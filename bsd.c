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

struct kevent cl[MAX_EVENTS];
struct kevent el[MAX_EVENTS];

struct timespec ts = { 0, 100000 };

Event
event(void* ptr)
{
	return (Event)ptr;
}

Event
poll_events()
{
	int n;
	Event retval = NULL;
	Event e = srv->ec;
	srv->ec = NULL;
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
		cl[n].udata = (void*)e;
		e = e->next;
	}
	n = kevent(srv->kq,cl,n,el,srv->numevents, &ts);
	if (n < 0) goto done;
	while (n--) 
		retval = queue_event(retval,el[n].ident,event(el[n].udata)->type,el[n].flags == EV_EOF ? EOF : NONE,event(el[n].udata)->data);
done:
	return retval;
}
