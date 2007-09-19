// Sockets.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "log.h"
#include "sockets.h"

#ifdef LINUX 
	extern void remove_epoll(int fd);
#endif

SocketInfo gsci = {0,0,0};

void
signal_handler(int sig)
{
	error("Received signal %i\n",sig);
}

void
socket_signal_handlers()
{
	signal(SIGPIPE,signal_handler);
}

int
nonblock(int fd)
{
	int flags = fcntl(fd,F_GETFL,0);
	return fcntl(fd,F_SETFL,flags | O_NONBLOCK);
}

int
open_socket(int  port)
{
	int one = 1;
	struct sockaddr_in addr;
	int fd = socket(AF_INET,SOCK_STREAM,0);
	if (0 > fd) return -1;
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));
	memset(&addr,0,sizeof(addr));
#ifndef LINUX
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	notice("Bind to port %i\n",port);	
	if (bind(fd,(struct sockaddr*)&addr,sizeof(struct sockaddr_in))) {
		error("Failed to bind to port %i\n",port);	
		close(fd);
		return -1;
	}
	if (listen(fd,128)) { // See man page on listen
		close(fd);
		return -1;
	}
	return fd;
}

Socket
accept_socket(Socket sc, int fd, TLSInfo tls)
{
	Socket retval;
	struct sockaddr_in saddr;
	socklen_t slen = sizeof(struct sockaddr_in);
	int sock = accept(fd,(struct sockaddr*)&saddr,&slen);
	if (sock < 1) {
		error("[JAWAS] failed to accept socket\n");
		return NULL;
	}
	nonblock(sock);
	retval = (Socket)salloc(sizeof(struct socket_cache_struct));
	memset(retval,0,sizeof(struct socket_cache_struct));
	retval->host = NULL;
	retval->tls = (tls ? open_tls(tls,sock) : NULL);
	retval->buf = NULL;
	retval->next = sc;
	retval->fd = sock;
	retval->peer = saddr.sin_addr.s_addr;
	retval->port = saddr.sin_port;
	retval->scratch = gscratch;
	++gsci.current;
	++gsci.total;
	gsci.max = max(gsci.current,gsci.max);
	return retval;
}

Socket
close_socket(Socket sc)
{
	if (!sc) return NULL;
	Socket retval = sc->next;
	if (sc->tls) close_tls(sc->tls);
#ifdef LINUX
	remove_epoll(sc->fd);
#endif
	close(sc->fd);
	free_scratch(sc->scratch);
	--gsci.current;
	return retval;
}

Socket
reset_socket(Socket sc)
{
	if (!sc) return NULL;
	free_scratch(sc->scratch);
	sc->scratch = new_scratch(NULL);
	return sc;
}

Buffer
read_socket(Socket sc)
{
	int bytes = 0;
	Buffer retval = sc->buf;
	for (retval = new_buffer(retval,(retval ? retval->pos + retval->length : 0)); 
		bytes = (sc->tls ? 
			read_tls(sc->tls,retval->data,Max_Buffer_Size) : 
			read(sc->fd,retval->data,Max_Buffer_Size)); 
		retval = new_buffer(retval,retval->pos + retval->length)) {
		if (bytes == -1 ) {
			if (errno == EAGAIN) {
				free_buffer(retval);
				return sc->buf;
			} else {
				return NULL;	
			}
		}
		retval->length = bytes;
		sc->buf = retval;
	}
	return NULL;
}

int
write_socket(Socket sc, char* src, int len)
{
	if (! sc) return 0;
	write(2,src,len);
	return (sc->tls ? write_tls(sc->tls,src,len) :write(sc->fd,src,len));
}
