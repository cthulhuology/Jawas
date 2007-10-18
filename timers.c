// timers.c
//
// Copyright (C) 2007 David J. Goehrig <dave@nexttolast.com>
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "headers.h"
#include "server.h"
#include "jws.h"
#include "timers.h"

Timers
add_timer(str file, int period, Headers data)
{
	server_scratch();	
	Timers retval = (Timers)salloc(sizeof(struct timer_struct));
	append_header(data,Str("timer"),Str("%p",retval));
	retval->next = srv->timers;
	retval->script = load(file);
	retval->period = period;
	retval->data = data;
	srv->timers = retval;
	client_scratch();
	return retval;
}

void
update_timers()
{
	Timers t;
	time_t now = time(NULL);
	if (now == srv->time) return;
	srv->time = now;
	++srv->period;
	for (t = srv->timers; t; t = t->next) 
		if (t->period == 0 || 0 == srv->period % t->period) 
			run_script(t->script,t->data);
}

void
cancel_timer(Timers t)
{
	Timers x;
	if (!t) return;
	if (srv->timers == t)  {
		srv->timers = srv->timers->next;
		return;
	}
	for (x = srv->timers; x; x = x->next) {
		if (x->next == t) {
			x->next = t->next;
			return;
		}
	}
}


