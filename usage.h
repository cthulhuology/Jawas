// usage.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __USAGE_H__
#define __USAGE_H__

typedef struct usage_struct* Usage;
struct usage_struct {
	int type;
	struct rusage start;
	struct rusage end;
	int requests;
	int hits;
	int time;
};

Usage new_usage(int type);
void start_usage(Usage u);
void stop_usage(Usage u);
void dump_usage();

#endif
