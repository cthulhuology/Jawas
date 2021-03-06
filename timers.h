// timers.h
//
// Copyright (C) 2007 David J. Goehrig <dave@nexttolast.com>
// All Rights Reserved
//

#ifndef __HAVE_TIMERS_H__
#define __HAVE_TIMERS_H__

extern struct itimerval Timeout;

void timeout(int seconds, int useconds);
void timer();

#endif
