// timers.c
//
// Copyright (C) 2007 David J. Goehrig <dave@nexttolast.com>
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "log.h"
#include "timers.h"

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
