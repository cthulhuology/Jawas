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

int today;
int log_fd = 2;
int log_level = LOG_LEVEL;

str log_path;
str index_path = NULL;
str localhost = NULL;

Scratch log_scratch_pad = NULL;

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
open_log()
{	
	log_scratch();
	struct stat st;
	int ol = 0;
	time_t now = time(NULL);
	struct tm* tms = localtime(&now);	
	tms->tm_sec = 0;
	tms->tm_min = 0;
	tms->tm_hour = 0;
	today = tms->tm_yday;
	now = mktime(tms);
	if (! localhost )
		localhost = char_str("localhost",0);
	if (! index_path) {
		index_path = char_str("/logs/index.html",0);
		index_path = file_path(localhost,index_path);
	}
	symlink(log_path->data,index_path->data);
	ol = stat(log_path->data,&st) < 0 ? 1 : 0;
	log_fd = open(log_path->data,O_WRONLY|O_APPEND|O_CREAT,0600);
	if (log_fd < 0) log_fd = 2;
	if (ol) write(log_fd,"<ol>\n",5);
	old_log_scratch();
}

time_t
rotate_logs()
{
	int i;
	time_t retval = time(NULL);
	struct tm* tms = localtime(&retval);
	if (tms->tm_yday != today) {
		write(log_fd,"</ol>",5);
		close_log();
		open_log();
	}
	return retval;
}

void
close_log()
{
	close(log_fd);
}

static int max_log_lvl = 2;
char* log_msgs[] =  {
	"[ERROR]",
	"[NOTICE]",
	"[DEBUG]",
	NULL
};

void
log_msg(int lvl, char* fmt,  ...)
{
	va_list args;
	time_t now = rotate_logs();
	if (lvl < 0 || lvl > log_level || lvl > max_log_lvl) return;
	log_scratch();
	va_start(args,fmt);
	str msg = new_str(fmt,args);
	char* ctm = ctime(&now);
	str tmp = char_str(ctm,strlen(ctm)-1);;
	msg = Str("<li>%c [%s]: %s</li>\n",log_msgs[lvl],tmp,msg);
	write(log_fd,msg->data,msg->len);
	fsync(log_fd);
	old_log_scratch();
}

