// timers.c
//
// Copyright (C) 2007 David J. Goehrig <dave@nexttolast.com>
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "headers.h"
#include "server.h"
#include "jws.h"
#include "timers.h"

Timers
add_timer(str file, time_t when, Headers data)
{
	server_scratch();	
	Timers retval = (Timers)salloc(sizeof(struct timer_struct));
	append_header(data,Str("timer"),Str("%p",retval));
	retval->next = srv->timers;
	retval->script = load(file);
	if (! retval->script) {
		error("Failed to start time for file %s",file);
		return NULL;
	}
	retval->when = when;
	debug("Current type is %i",srv->time);
	debug("Scheduling for %i (%i from now)",when, when - srv->time);
	retval->data = data;
	srv->timers = retval;
	client_scratch();
	return retval;
}

void
update_timers()
{
	Timers t,x;
	time_t now = time(NULL);
	if (now == srv->time) return;
	srv->time = now;
	t = srv->timers;
	srv->timers = NULL;
	while (t) {
		if (t->when <= srv->time) {
			run_script(t->script,t->data);
			t = t->next;
		} else  {
			x = t;
			t = t->next;
			x->next = srv->timers;
			srv->timers = x;
		}
	}
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

void
init_timers()
{
	Headers hds = new_headers();
	append_header(hds,Str("started"),Str("0"));
	struct dirent* de;
	str dotjs = Str(".js");
	str file;
	DIR* d = opendir("startup");
	if (! d) return;
	while (NULL != (de = readdir(d))) {
		file = char_str(de->d_name + de->d_namlen - 3, 3);
		debug("Starting timer %s", file);
		if (de->d_namlen > 3 && ncmp_str(dotjs,file,3))
			add_timer(Str("startup/%c",de->d_name),0,hds);
	}
}
