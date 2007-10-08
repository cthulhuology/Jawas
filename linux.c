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

#define NODE_FLAGS DN_MODIFY | DN_DELETE | DN_RENAME | DN_ATTRIB

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
			ep.events = EPOLLIN | (ec->flag == ONESHOT ? EPOLLET | EPOLLONESHOT: 0);
			ep.data.ptr = ec->fd == srv->http_sock ? &srv->http_sock : ec->fd == srv->tls_sock ? &srv->tls_sock : ec->data;
			if (epoll_ctl(KQ,EPOLL_CTL_ADD,ec->fd,&ep))
				if (epoll_ctl(KQ,EPOLL_CTL_MOD,ec->fd,&ep))
					debug("Failed to add event %i for fd %i", ec->type,ec->fd);
			break;
		case WRITE:
			ep.events = EPOLLOUT | (ec->flag == ONESHOT ? EPOLLET| EPOLLONESHOT : 0);
			ep.data.ptr = ec->data;
			debug("Adding WRITE fd %i  data %p",ep.data.fd,ep.data.ptr);
			if (epoll_ctl(KQ,EPOLL_CTL_ADD,ec->fd,&ep))
				if (epoll_ctl(KQ,EPOLL_CTL_MOD,ec->fd,&ep))
					debug("Failed to add event %i for fd %i", ec->type,ec->fd);
			break;
		case NODE:  // NB: Add node events here, not handled by epoll, but by dnotify and SIGIO
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
		if (el[n].data.ptr == &srv->http_sock) {
			debug("Processing incomming HTTP connection on socket %i", srv->http_sock);
			retval = queue_event(retval,srv->http_sock,READ,NONE,NULL);
		} else if (el[n].data.ptr == &srv->tls_sock) {
			debug("Processing incomming TLS connection on socket %i",srv->tls_sock);
			retval = queue_event(retval,srv->tls_sock,READ,NONE,NULL);
		} else {
			if (el[n].events & EPOLLIN) { // READ events are Requests
				Request req = (Request)el[n].data.ptr;
				debug("Processing request %p on socket %i",req, req->sc->fd);
				retval = queue_event(retval,req->sc->fd,READ, NONE,req);	
			} else { // WRITE events are Responses
				Response resp = (Response)el[n].data.ptr;
				debug("Processing response %p on socket %i",resp, resp->sc->fd);
				retval = queue_event(retval,resp->sc->fd,WRITE, NONE,resp);	
			} // NB: NODE are handled above!
		}
	}
done:
	free_scratch(scratch);	
	free_scratch(tmp);
	return retval;
}

void
add_file_monitor(int f,void* r)
{
	if(fcntl(f,F_NOTIFY,NODE_FLAGS))
		debug("Failed to monitor file %i",f);
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

void
remove_epoll(int fd)
{
	struct epoll_event ep;
	ep.events = EPOLLIN | EPOLLOUT;
	ep.data.fd = fd;
	if (epoll_ctl(KQ,EPOLL_CTL_DEL,fd,&ep))
		debug("Failed to clear fd %i",fd);
}

#endif
