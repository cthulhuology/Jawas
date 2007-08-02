// log.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "uri.h"
#include "index.h"
#include "log.h"

int today;
int log_fd = 2;
int log_level = LOG_LEVEL;

str log_path;
str index_path = NULL;
str localhost = NULL;

void
open_log()
{	
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
cstr log_msgs[] =  {
	{ 8, "[ERROR] " },
	{ 9, "[NOTICE] " },
	{ 8, "[DEBUG] " },
	{ 0, NULL },
};

void
log_msg(int lvl, char* fmt,  ...)
{
	char* tmp;
	va_list args;
	time_t now = rotate_logs();
	if (lvl < 0 || lvl > log_level || lvl > max_log_lvl) return;
	write(log_fd,"<li>",4);
	write(log_fd,log_msgs[lvl].data,log_msgs[lvl].len);
	if (log_fd != 2) 
		write(2,log_msgs[lvl].data,log_msgs[lvl].len);
	tmp = ctime(&now);
	write(log_fd,tmp,strlen(tmp)-1);
	if (log_fd != 2) 
		write(log_fd,tmp,strlen(tmp)-1);
	write(log_fd,": ",2);
	if (log_fd != 2) 
		write(log_fd,": ",2);
	va_start(args,fmt);
	str msg = new_str(fmt,args);
	write(log_fd,msg->data,msg->len);
	if (log_fd != 2) 
		write(log_fd,msg->data,msg->len);
	write(log_fd,"</li>\n",6);
	fsync(log_fd);
}

