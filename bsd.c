// bsd.c
// 
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "events.h"
#include "server.h"

#ifndef LINUX

extern Scratch escratch;

Event
poll_events(Event ec, int numevents)
{
	int n;
	Scratch scratch = new_scratch(NULL);
	Scratch tmp = escratch;
	escratch = NULL;
	Event retval = NULL;
	struct timespec ts = { 0, 0 };
	struct kevent* cl = (struct kevent*)(ec ? alloc_scratch(scratch,sizeof(struct kevent)*ec->pos) : NULL);
	struct kevent* el = (struct kevent*)alloc_scratch(scratch,sizeof(struct kevent)*numevents);

	for (n = 0; ec; ++n) {
		cl[n].ident = ec->fd;
		cl[n].filter = (ec->type == READ ? EVFILT_READ :
				ec->type == WRITE ? EVFILT_WRITE :
				ec->type == NODE ? EVFILT_VNODE : 0);
		cl[n].flags = EV_ADD | (ec->flag == ONESHOT ? EV_ONESHOT : 0);;
		cl[n].fflags = (ec->type == NODE ? NODE_FLAGS : 0);
		cl[n].data = 0;
		cl[n].udata = ec->data;
		ec = ec->next;
	}
	n = kevent(KQ,cl,n,el,numevents, NULL);
	if (n < 0) goto done;
	debug("Processing %i events",n);
	while (n--) 
		retval = queue_event(retval,el[n].ident,el[n].filter == EVFILT_READ ? READ : el[n].filter == EVFILT_WRITE ? WRITE : NODE, el[n].flags == EV_EOF ? EOF : NONE,el[n].udata);
done:
	free_scratch(scratch);
	free_scratch(tmp);
	return retval;
}

#endif
