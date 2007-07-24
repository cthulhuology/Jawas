// log.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_LOG_H__
#define __HAVE_LOG_H__

typedef struct str_struct str;
struct str_struct {
	int len;
	char* data;
};

void open_log();
void close_log();

void log_msg(int lvl, char* fmt, ...);

extern int log_level;

#endif

