// timers.h
//
// Copyright (C) 2007 David J. Goehrig <dave@nexttolast.com>
// All Rights Reserved
//

#ifndef __HAVE_TIMERS_H__
#define __HAVE_TIMERS_H__

#include "str.h"
#include "headers.h"
#include "alloc.h"

typedef struct timer_struct* Timers;
struct timer_struct {
	Timers next;
	Scratch scratch;
	File script;
	Headers data;
	time_t when;
};

Timers add_timer(str file, time_t when);
Timers set_timer_value(Timers t, str key, str value);
void cancel_timer(Timers t);
void update_timers();
void init_timers();

#endif
