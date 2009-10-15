// timers.c
//
// Copyright (C) 2007 David J. Goehrig <dave@nexttolast.com>
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "log.h"
#include "headers.h"
#include "server.h"
#include "jws.h"
#include "timers.h"

Scratch timer_old_scratch;
Scratch timer_scratch;

Timers
add_timer(str file, time_t when)
{
	timer_old_scratch = gscratch;
	timer_scratch = new_scratch(NULL);
	set_scratch(timer_scratch);
	Timers retval = (Timers)salloc(sizeof(struct timer_struct));
	retval->scratch = timer_scratch;
	retval->next = srv->timers;
	retval->script = load(file);
	if (! retval->script) {
		error("Failed to start time for file %s",file);
		set_scratch(timer_old_scratch);
		return NULL;
	}
	retval->when = when + srv->time;
	retval->data = new_headers();
	append_header(retval->data,Str("timer"),Str("%p",retval));
	srv->timers = retval;
	set_scratch(timer_old_scratch);
	return retval;
}

Timers
set_timer_value(Timers t, str key, str value)
{
	if (!t || ! t->data) return NULL;
	timer_old_scratch = gscratch;
	set_scratch(t->scratch);
	append_header(t->data,Str("%s",key),Str("%s",value));
	set_scratch(timer_old_scratch);
	return t;
}

void
update_timers()
{
	Timers t,x;
	time_t now = time(NULL);
	if (now == srv->time) return;
	srv->time = now;
//	debug("Update Timers %i",now);
	t = srv->timers;
	srv->timers = NULL;
	while (t) {
		if (t->when <= srv->time) {
//			debug("Running %p for time %i",t,t->when);
			timer_old_scratch = gscratch;
			set_scratch(t->scratch);
			run_script(t->script,t->data);
//			debug("Ran script for timer %p",t);
			set_scratch(timer_old_scratch);
			x = t;
			t = t->next;
			free_scratch(x->scratch);
		} else  {
//			debug("Queing %p for time %i",t,t->when);
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
		free_scratch(t->scratch);
		return;
	}
	for (x = srv->timers; x; x = x->next) {
		if (x->next == t) {
			x->next = t->next;
			free_scratch(t->scratch);
			return;
		}
	}
}

void
init_timers()
{
	str file, dotjs = Str(".js");
	struct dirent* de;
	DIR* d = opendir("startup");
	if (! d) return;
	while (NULL != (de = readdir(d))) {
#ifdef LINUX
		int len = strlen(de->d_name);
#else
		int len = de->d_namlen;
#endif
		if (len < 3) continue;
		file = copy(&de->d_name[len - 3], 0);
		if (len > 3 && ncmp(dotjs,file,3)) {
			set_timer_value(add_timer(Str("startup/%c",de->d_name),0),Str("started"),Str("0"));
		}
	}
}

struct itimerval Timeout;
void
timeout(int seconds, int useconds) {
	Timeout.it_interval.tv_usec = 0;
	Timeout.it_interval.tv_sec = 0;
	Timeout.it_value.tv_usec = useconds;
	Timeout.it_value.tv_sec = seconds;
}

void
timer()
{
	if (setitimer(0, &Timeout, &Timeout))
		error("Failed to set timeout");
}
