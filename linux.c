// linux.c
// 
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//


#include "include.h"
#include "defines.h"
#include "events.h"
#include "server.h"

#ifdef LINUX

extern Scratch escratch;

Event
poll_events(Event ec, int numevents)
{
	int n;
	Scratch scratch = new_scratch(NULL);
	Scratch tmp = escratch;
	escratch = NULL;
	Event retval = NULL;
	struct epoll_event ep;

	for (n = 0; ec; ++n) {
		switch(ec->type) {
		case READ:
			ep.events = EPOLLIN; //  | EPOLLET |(ec->flag == ONESHOT ? EPOLLONESHOT : 0);
			ep.data.ptr = ec->data;
			debug("Adding READ fd %i  data %p",ep.data.fd,ep.data.ptr);
			if (epoll_ctl(KQ,EPOLL_CTL_ADD,ec->fd,&ep))
				debug("Failed to add event %i for fd %i", ec->type,ec->fd);
			break;
		case WRITE:
			ep.events = EPOLLOUT |EPOLLET | (ec->flag == ONESHOT ? EPOLLONESHOT : 0);
			ep.data.ptr = ec->data;
			debug("Adding WRITE fd %i  data %p",ep.data.fd,ep.data.ptr);
			if (epoll_ctl(KQ,EPOLL_CTL_MOD,ec->fd,&ep))
				debug("Failed to add event %i for fd %i", ec->type,ec->fd);
			break;
		case NODE:
			retval = queue_event(retval,ec->fd,ec->type,ec->flag,ec->data);
			break;
		}
		ec = ec->next;
	}
	struct epoll_event* el  = (struct epoll_event*)alloc_scratch(scratch,sizeof(struct epoll_event)*numevents);
	debug("numevents is %i", numevents);
	n = epoll_wait(KQ,el,numevents,-1);
	if (n < 0) goto done;
	debug("Epoll got %i events",n);
	while (n--) {
		debug("Processing index %i events %i fd %i",n, el[n].events,el[n].data.fd);
		retval = queue_event(retval,el[n].data.fd,el[n].events & EPOLLIN ? READ : el[n].events & EPOLLOUT ? WRITE : NODE,NONE,el[n].data.ptr);
	}
done:
	free_scratch(scratch);	
	free_scratch(tmp);
	return retval;
}

void
linux_file_monitor(int fd)
{
	if(fcntl(fd,F_NOTIFY,NODE_FLAGS))
		debug("Failed to monitor file %i",fd);
}

void
dnotify_handler(int sig,siginfo_t* info,void* u)
{
	debug("File %i activated",info->si_fd);
	srv->ec = queue_event(srv->ec,info->si_fd,NODE, NONE,query_fd_cache(srv->fc,info->si_fd)); 
}

void
file_signal_handlers()
{
	sigset_t set;
	sigemptyset(&set);
	struct sigaction act;
	act.sa_sigaction = dnotify_handler;
	act.sa_flags = SA_SIGINFO;
	act.sa_mask = set;
	sigaction(SIGIO,&act,NULL);
}

#endif
