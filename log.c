// log.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "server.h"
#include "index.h"
#include "log.h"
#include "dates.h"

Scratch log_scratch_pad = NULL;

int log_fd = 2;
int log_level = LOG_LEVEL;

const int max_log_lvl = 2;
char* log_msgs[] =  {
	"[ERROR]",
	"[DB]",
	"[NOTICE]",
	"[DEBUG]",
	NULL
};

void
log_scratch()
{
	log_scratch_pad = gscratch;	
	set_scratch(new_scratch(NULL));
}

void
old_log_scratch()
{
	free_scratch(gscratch);
	set_scratch(log_scratch_pad);	
}

void
log_msg(int lvl, char* fmt,  ...)
{
	va_list args;
	if (lvl < 0 || lvl > log_level || lvl > max_log_lvl) return;
	log_scratch();
	va_start(args,fmt);
	str msg = new_str(fmt,args);
	str now = Date();
	msg = Str("%c [%s]: %s\n",log_msgs[lvl],now,msg);
	char* msg_data = dump(msg);
	write(log_fd,msg_data,len(msg));
	fsync(log_fd);
	free(msg_data);
	old_log_scratch();
}

