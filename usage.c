// usage.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "usage.h"

int
timediff(struct timeval* tva,struct timeval* tvb)
{
	int sec = tva->tv_sec - tvb->tv_sec;
	int usec = tva->tv_usec - tvb->tv_usec;
	return (sec << 20) + usec;
}

Usage
new_usage(int t) 
{
	Usage retval =  (Usage)salloc(sizeof(struct usage_struct));
	retval->type = t;
	return retval;
}

void
start_usage(Usage u)
{
	if (getrusage(RUSAGE_SELF,&u->start)) 
		error("Failed to calculate usage");
}

void
stop_usage(Usage u)
{
	if (getrusage(RUSAGE_SELF,&u->end)) 
		error("Failed to calculate usage");
	int delta = timediff(&u->end.ru_utime,&u->start.ru_utime) + timediff(&u->end.ru_stime,&u->start.ru_stime);
	debug("Delta is %i",delta);
	++u->requests;
	if (u->hits < 1000) 
		++u->hits;
	u->time = ((u->hits-1) * u->time + delta) / u->hits;
}

void
dump_usage(Usage u)
{
	debug("USAGE: %c",u->type ? "Server" : "Request");
	notice("Requests: %i", u->requests);
	notice("Avg Time: %i", u->time);
	notice("Window Size: %i",u->hits);
	debug("Total Time: %i", timediff(&u->end.ru_utime,&u->start.ru_utime) + timediff(&u->end.ru_stime,&u->start.ru_stime));
	debug("\tUser Time: %i", timediff(&u->end.ru_utime,&u->start.ru_utime));
	debug("\tSystem Time: %i", timediff(&u->end.ru_stime,&u->start.ru_stime));
}


