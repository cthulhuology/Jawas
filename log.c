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
char* log_path;

void
open_log()
{	
	struct stat st;
	int i, ol = 0;
	time_t now = time(NULL);
	char* buffer;
	char* index_buffer;
	char* index_path;
	char* localhost = "localhost";
	int buflen, index_buflen;
	struct tm* tms = localtime(&now);	
	tms->tm_sec = 0;
	tms->tm_min = 0;
	tms->tm_hour = 0;
	today = tms->tm_yday;
	now = mktime(tms);
	buflen = asprintf(&buffer,"/logs/%d.html",now);
	log_path = file_path(localhost,9,buffer,buflen);	
	index_buflen = asprintf(&index_buffer,"/logs/index.html");
	index_path = file_path(localhost,9,index_buffer,index_buflen);
	symlink(log_path,index_path);
	ol = stat(log_path,&st) < 0 ? 1 : 0;
	log_fd = open(log_path,O_WRONLY|O_APPEND|O_CREAT,0600);
	if (log_fd < 0) log_fd = 2;
	if (ol) write(log_fd,"<ol>\n",5);
	free(buffer);
	free(index_buffer);
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
str log_msgs[] =  {
	{ 8, "[ERROR] " },
	{ 9, "[NOTICE] " },
	{ 8, "[DEBUG] " },
	{ 0, NULL },
};

void
log_msg(int lvl, char* fmt,  ...)
{
	va_list args;
	int i, tmpl, l;
	char* tmp;
	double dtmp;
	int itmp;
	time_t now = rotate_logs();
	if (lvl < 0 || lvl > log_level || lvl > max_log_lvl) return;
 	l = strlen(fmt);
	write(log_fd,"<li>",4);
	write(log_fd,log_msgs[lvl].data,log_msgs[lvl].len);
	tmp = ctime(&now);
	write(log_fd,tmp,strlen(tmp)-1);
	write(log_fd,": ",2);
	va_start(args,fmt);
	for (i = 0; i < l; ++i) {
		if (fmt[i] == '%') {
			switch(fmt[++i]) {
			case 's':
				tmp = va_arg(args,char*);
				write(log_fd,tmp,strlen(tmp));
				break;	
			case 'n':
				dtmp = va_arg(args,double);
				tmpl = asprintf(&tmp,"%g",dtmp);
				write(log_fd,tmp,tmpl);
				free(tmp);	
				break;
			case 'i':
				itmp = va_arg(args,int);
				tmpl = asprintf(&tmp,"%d",itmp);
				write(log_fd,tmp,tmpl);
				free(tmp);	
				break;
			case 'p':
				itmp = va_arg(args,int);
				tmpl = asprintf(&tmp,"%p",itmp);
				write(log_fd,tmp,tmpl);
				free(tmp);	
				break;
			default:
				write(log_fd,&fmt[i],1);
				break;
			}
		} else {
			write(log_fd,&fmt[i],1);
		}
	}
	write(log_fd,"</li>\n",6);
	fsync(log_fd);
	va_end(args);
}

