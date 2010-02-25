// log.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "server.h"
#include "index.h"
#include "log.h"
#include "dates.h"

int log_fd = 2;
int log_level = LOG_LEVEL;

const int max_log_lvl = 3;
char* log_msgs[] =  {
	"[ERROR]",
	"[DB]",
	"[NOTICE]",
	"[DEBUG]",
	NULL
};

void
log_msg(int lvl, char* fmt,  ...)
{
	va_list args;
	if (lvl < 0 || lvl > log_level || lvl > max_log_lvl) return;
	va_start(args,fmt);
	str msg = new_str(fmt,args);
	str now = Date(time(NULL));
	msg = _("%c [%s]: %s\n",log_msgs[lvl],now,msg);
	write(log_fd,msg->data,len(msg));
	fsync(log_fd);
//	vfprintf(stderr,fmt,args);
//	fprintf(stderr,"\n");
}

