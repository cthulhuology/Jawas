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

struct timespec ts = { 0, 100000 };

#ifdef BITS64

reg
add_event(reg q, reg fd, enum event_types t, enum event_flags f)
{
	struct kevent64_s ke;
	EV_SET64(&ke,fd,(t == READ || t == RESP ? EVFILT_READ :
		t == WRITE || t == REQ ? EVFILT_WRITE :
		t == PROC ? EVFILT_PROC :
		t == NODE ? EVFILT_VNODE : 0),
		EV_ADD | (f == ONESHOT ? EV_ONESHOT : 0),
		(f == EXIT ? NOTE_EXIT : 0), 0, 0, 0, 0);
	return kevent64(q,&ke,1,NULL,0,0, &ts);
}

#else

reg
add_event(reg q, reg fd, enum event_types t, enum event_flags f)
{
	struct kevent ke;
	EV_SET(&ke,fd,(t == READ || t == RESP ? EVFILT_READ :
		t == WRITE || t == REQ ? EVFILT_WRITE :
		t == PROC ? EVFILT_PROC :
		t == NODE ? EVFILT_VNODE : 0),
		EV_ADD | (f == ONESHOT ? EV_ONESHOT : 0),
		(f == EXIT ? NOTE_EXIT : 0), 0);
	return kevent(q,&ke,1,NULL,0,&ts);
}

#endif


#ifdef BITS64

void
poll_events(reg q, function f)
{
	struct kevent64_s ke[MAX_EVENTS];
	int n = kevent64(q,NULL,0,ke,MAX_EVENTS,0,NULL);
	if (n <= 0) return;
	for (int i = 0; i < n; ++i) 
		f(ke[i].ident,ke[i].filter == EVFILT_PROC ? PROC : NONE);	
}

#else

void
poll_events(reg q, function f)
{
	struct kevent ke[MAX_EVENTS];
	int n = kevent(q,NULL,0,ke,MAX_EVENTS,NULL);
	if (n <= 0) return NULL;
	for (int i = 0; i < n; ++i ) 
		f(ke[i].ident,ke[i].filter == EVFILT_PROC ? PROC : NONE);	
}

#endif
