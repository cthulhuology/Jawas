// log.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_LOG_H__
#define __HAVE_LOG_H__

#include "str.h"

void open_log();
void close_log();

void log_msg(int lvl, char* fmt, ...);

extern int log_level;

#endif

