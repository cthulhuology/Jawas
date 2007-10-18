// timers.h
//
// Copyright (C) 2007 David J. Goehrig <dave@nexttolast.com>
// All Rights Reserved
//

#ifndef __HAVE_TIMERS_H__
#define __HAVE_TIMERS_H__

#include "str.h"

typedef struct timer_struct* Timers;
struct timer_struct {
	Timers next;
	File script;
	Headers data;
	int period;
};

Timers add_timer(str file, int period, Headers data);
void update_timers();
void cancel_timer(Timers t);

#endif
