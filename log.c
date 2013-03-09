// log.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "log.h"
#include "dates.h"

int log_fd = 2;
int log_level = LOG_LEVEL;

const int max_log_lvl = 4;
int log_msgs[] =  {
	LOG_ERR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_DEBUG,
	0
};

void
log_start() 
{
	openlog("jawas", LOG_NDELAY|LOG_CONS|LOG_PID, LOG_LOCAL6);
}

void
log_msg(int lvl, char* fmt,  ...)
{
	va_list args;
	if (lvl < 0 || lvl > log_level || lvl > max_log_lvl) return;
	va_start(args,fmt);
	str msg = new_str(fmt,args);
	syslog(log_msgs[lvl],msg->data);
	va_end(args);
}

