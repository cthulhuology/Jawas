// usage.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"

struct rusage req_usage_start;
struct rusage req_usage_end;

struct usage_struct {
	int requests;
	int total_time;	
	int avg_time;
	int window_size;
	int window_time;
} usage_stats;

int
timediff(struct timeval* tva,struct timeval* tvb)
{
	int sec = tva->tv_sec - tvb->tv_sec;
	int usec = tva->tv_usec - tvb->tv_usec;
	return (sec << 20) + usec;
}

void
start_usage()
{
	if (getrusage(RUSAGE_SELF,&req_usage_start)) 
		error("Failed to calculate usage");
}

void
stop_usage()
{
	if (getrusage(RUSAGE_SELF,&req_usage_end)) 
		error("Failed to calculate usage");
	int delta = timediff(&req_usage_end.ru_utime,&req_usage_start.ru_utime) + timediff(&req_usage_end.ru_stime,&req_usage_start.ru_stime);
	++usage_stats.requests;
	usage_stats.total_time += delta;
	usage_stats.avg_time = usage_stats.total_time / usage_stats.requests;
	if (usage_stats.window_size < 1000) ++usage_stats.window_size;
	usage_stats.window_time = ((usage_stats.window_size-1) * usage_stats.window_time + delta) / usage_stats.window_size;
}

void
dump_usage()
{
	debug("USAGE:");
	debug("Total Time: %i", timediff(&req_usage_end.ru_utime,&req_usage_start.ru_utime) + timediff(&req_usage_end.ru_stime,&req_usage_start.ru_stime));
	debug("\tUser Time: %i", timediff(&req_usage_end.ru_utime,&req_usage_start.ru_utime));
	debug("\tSystem Time: %i", timediff(&req_usage_end.ru_stime,&req_usage_start.ru_stime));
	debug("Page Faults: %i", req_usage_end.ru_majflt - req_usage_start.ru_majflt);
	debug("Swaps: %i", req_usage_end.ru_nswap - req_usage_start.ru_nswap);
	debug("Blocks In: %i", req_usage_end.ru_inblock - req_usage_start.ru_inblock);
	debug("Blocks Out: %i", req_usage_end.ru_oublock - req_usage_start.ru_oublock);
	debug("Context Switches: %i", req_usage_end.ru_nivcsw + req_usage_end.ru_nvcsw - req_usage_start.ru_nivcsw - req_usage_start.ru_nvcsw);
	debug("\tInvoluntary Switches: %i", req_usage_end.ru_nivcsw - req_usage_start.ru_nivcsw);
	debug("\tVoluntary Switches: %i", req_usage_end.ru_nvcsw - req_usage_start.ru_nvcsw);

	notice("USAGE SUMMARY:");
	notice("Requests: %i", usage_stats.requests);
	notice("Total Time: %i", usage_stats.total_time);
	notice("Avg Time: %i", usage_stats.avg_time);
	notice("Window Time: %i",usage_stats.window_time);
	notice("Window Size: %i",usage_stats.window_size);
}


