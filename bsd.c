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

#ifndef LINUX

#define NODE_FLAGS NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE

extern Scratch escratch;

struct kevent* cl = NULL;
struct kevent* el = NULL;

Event
poll_events(Event ec, int numevents)
{
	int n;
	Scratch tmp = escratch;
	escratch = new_scratch(NULL);
	set_scratch(escratch);
	Event retval = NULL;
	EventData data;
	struct timespec ts = { 1, 0 };
	cl = (!cl ? (struct kevent*)malloc(sizeof(struct kevent)*255) : cl);
	el = (!el ? (struct kevent*)malloc(sizeof(struct kevent)*255) : el);
	memset(cl,0,sizeof(struct kevent)*255);
	memset(el,0,sizeof(struct kevent)*255);
	for (n = 0; ec; ++n) {
		cl[n].ident = ec->fd;
		cl[n].filter = (ec->type == READ || ec->type == RESP ? EVFILT_READ :
				ec->type == WRITE || ec->type == REQ ? EVFILT_WRITE :
				ec->type == NODE ? EVFILT_VNODE : 0);
		cl[n].flags = EV_ADD | (ec->flag == ONESHOT ? EV_ONESHOT : 0);;
		cl[n].fflags = (ec->type == NODE ? NODE_FLAGS : 0);
		cl[n].data = 0;
		cl[n].udata = ec->data;
		debug("[kevent %i on %i]",cl[n].filter,cl[n].ident);
		ec = ec->next;
	}
	n = kevent(KQ,cl,n,el,numevents, &ts);
	if (n < 0) goto done;
	while (n--)  {
		data = (EventData)el[n].udata;
		retval = queue_event(retval,el[n].ident,event_type(data),el[n].flags == EV_EOF ? EOF : NONE,event_data(data));
	}
done:
	free_scratch(tmp); // Done with old event scratch freeing
	return retval;
}

void
add_file_monitor(int f, void* r)
{
	srv->ec = queue_event(srv->ec,f, NODE, ONESHOT, r);
	srv->numevents++;
}

#endif
