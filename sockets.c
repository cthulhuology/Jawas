// Sockets.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "str.h"
#include "log.h"
#include "sockets.h"
#include "server.h"

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
	signal(SIGPIPE,SIG_IGN);
}

int
nonblock(int fd)
{
	int flags = fcntl(fd,F_GETFL,0);
	return fcntl(fd,F_SETFL,flags | O_NONBLOCK);
}

int
set_timeout(Socket sc)
{
	struct itimerval timeout;
	timeout.it_interval.tv_usec = 0;
	timeout.it_interval.tv_sec = 0;
	timeout.it_value.tv_usec = 0;
	timeout.it_value.tv_sec = SOCKET_CONNECT_TIMEOUT;
	return setsockopt(sc->fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(struct itimerval))
		&& setsockopt(sc->fd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(struct itimerval));
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
		error("[JAWAS] failed to accept socket");
		return NULL;
	}
	nonblock(sock);
	retval = (Socket)salloc(sizeof(struct socket_cache_struct));
	retval->host = NULL;
	retval->tls = (tls ? open_tls(tls,sock) : NULL);
	retval->buf = NULL;
	retval->next = sc;
	retval->fd = sock;
	retval->peer = saddr.sin_addr.s_addr;
	retval->port = saddr.sin_port;
	retval->scratch = gscratch;
	retval->closed = 0;
	++gsci.current;
	++gsci.total;
	gsci.max = max(gsci.current,gsci.max);
	return retval;
}

Socket
connect_socket(char* host, int port)
{
	Socket retval;
	int i;
	struct hostent* hst = gethostbyname(host);;
	struct sockaddr_in saddr;
	struct in_addr **list;
	socklen_t slen = sizeof(struct sockaddr_in);
	struct itimerval timeout;
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock < 1) {	
		error("[JAWAS] failed to create socket");
		return NULL;
	}
	if (!hst) {
		error("[JAWAS] failed to lookup %s",host);
		return NULL;
	}
	list = (struct in_addr**)hst->h_addr_list;
	int connected = 0; 
	for (i = 0; list[i]; ++i) {
		saddr.sin_port = htons(port);
		saddr.sin_addr = *list[i];
		saddr.sin_family = AF_INET;
		debug("Connecting to %c",inet_ntoa(*list[i]));
		timeout.it_interval.tv_usec = 0;
		timeout.it_interval.tv_sec = 0;
		timeout.it_value.tv_usec = 0;
		timeout.it_value.tv_sec = SOCKET_CONNECT_TIMEOUT;
		if (setitimer(0, &timeout, &timeout))
			error("Failed to set timeout");
		connected = 1;
		if (connect(sock,(struct sockaddr*)&saddr,slen)) {
			if (srv->alarm) {
				error("[JAWAS] socket conect timeout!");
				srv->alarm = 0;
			}
			perror("connect");
			error("[JAWAS] failed to connect to %c:%i",host,port);
			connected = 0;
			continue;
		}
		srv->alarm = 0;
		timeout.it_interval.tv_usec = 0;
		timeout.it_interval.tv_sec = 0;
		timeout.it_value.tv_usec = 0;
		timeout.it_value.tv_sec = 0;
		if (setitimer(0,&timeout,&timeout)) 
			error("Failed to clear timeout");
		break;
	}
	if (!connected) return NULL;
	nonblock(sock);
	retval = (Socket)salloc(sizeof(struct socket_cache_struct));
	retval->next = NULL;
	retval->fd = sock;
	retval->scratch = new_scratch(NULL);
	retval->buf = NULL;
	retval->tls = NULL;
	retval->port = port;
	retval->host = Str("%c",host);
	retval->peer = (int)saddr.sin_addr.s_addr;
	retval->closed = 0;
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

str
read_socket(Socket sc)
{
	str retval = sc->buf,t;
	for (t = blank(Max_Buffer_Size); (t->length = sc->tls ? 
			read_tls(sc->tls,t->data,Max_Buffer_Size) : 
			read(sc->fd,t->data,Max_Buffer_Size)); sc->buf = append(sc->buf,t)) {
		debug("Length read %i [%s], length contents %i",t->length,t, len(retval));
		if (t->length == -1 ) {
			if (errno == EAGAIN) {
				debug("EAGAIN");
				debug("Length of sc buf %i",len(sc->buf));
				return sc->buf;
			} else {
				error("ERROR %i occured", errno);
				return NULL;	
			}
		}
	}
	return sc->buf;
}

int
write_to_socket(Socket sc,char* data, int length)
{
	debug("Writing [%s]",copy(data,length));
	if (sc->closed) return 0;
	int retval = sc->tls ? 
		write_tls(sc->tls,data,length) :
		write(sc->fd,data,length);
	if (retval < 0) {
		sc->closed = 1;
		return 0;
	}
	return retval;
}

int
write_socket(Socket sc, str buf)
{
	str t;
	int retval = 0;
	if (! sc) return 0;
	for (t = buf; t; t = buf->next) 
		retval += write_to_socket(sc,t->data,t->length);
	return retval;
}

int
write_chunk(Socket sc, char* data, int length)
{
	int retval = 0;
	str header = Str("%h\r\n",length);
	fprintf(stderr,"Writing chunk %i\n",length);
	write_to_socket(sc,header->data,header->length);
	fprintf(stderr,"Header length %i",header->length);
	if (data) 
		retval = write_to_socket(sc,data,length);
	write_to_socket(sc,"\r\n",2);
	return retval;
}

int
write_chunked_socket(Socket sc, str buf)
{
	int retval = 0;
	if (! sc) return 0;
	char* data = dump(buf);
	int i, l = len(buf);
	for (i = 0; i < l; i += MAX_WRITE_SIZE)
		retval += write_chunk(sc,data + i,min(MAX_WRITE_SIZE,l-i));
	return retval;
}

int
send_contents(Socket sc, str buf, int chunked)
{
	if (!sc || !buf) return 0;
	fprintf(stderr," Writing chunked? %i\n",chunked);
	return chunked ? 
		write_chunked_socket(sc,buf) : 
		write_socket(sc,buf);
}

int
write_raw_chunked_socket(Socket sc, char *data, int length)
{
	int retval = write_chunk(sc,data,length);		
	if (retval < 0) {
		retval = 0;
		sc->closed = 1;
	}
	return retval;
}

int
send_raw_contents(Socket sc, File fc, int off)
{
	if (!sc || !fc) return 0;
	return write_raw_chunked_socket(sc,fc->data+off,min(fc->st.st_size-off,MAX_WRITE_SIZE));
}

int
closed_socket(Socket sc, char* msg)
{
	if (sc->closed) {
		error("%c : Socket<%p>",msg);
		return 1;
	}
	return 0;
}

void
socket_notice(Socket sc, char* msg)
{
	notice("Socket <%p> %i.%i.%i.%i:%i %c\n",
		srv->sc,
		(0xff & srv->sc->peer),
		(0xff00 & srv->sc->peer) >> 8,
		(0xff0000 & srv->sc->peer) >> 16,
		(0xff000000 & srv->sc->peer) >> 24,
		srv->sc->port, msg);
}
