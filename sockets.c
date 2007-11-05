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
	signal(SIGPIPE,signal_handler);
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
	retval = (Socket)salloc(sizeof(struct socket_cache_struct));
	retval->next = NULL;
	retval->fd = sock;
	retval->scratch = NULL;
	retval->buf = NULL;
	retval->tls = NULL;
	retval->port = port;
	retval->host = Str("%c",host);
	retval->peer = (int)saddr.sin_addr.s_addr;
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
	if (sc->tls) {
		// write(2,src,len);
		return write_tls(sc->tls,src,len);
	} else {
		// write(2,src,len);
		return write(sc->fd,src,len);
	}
}

int
write_chunked_socket(Socket sc, char* src, int len)
{
	int total;
	str header = Str("%h\r\n",len);
	if (! sc) return 0;
	if (sc->tls) {
		write_tls(sc->tls,header->data,header->len);
		if (src) total = write_tls(sc->tls,src,len);
		write_tls(sc->tls,"\r\n",2);
	} else {
		// write(2,header->data,header->len);
		write(sc->fd,header->data,header->len);
 		// if (src) total = write(2,src,len);
 		if (src) total = write(sc->fd,src,len);
		// write(2,"\r\n",2);
		write(sc->fd,"\r\n",2);
	}
	return total;
}

str
readstr_socket(Socket sc)
{
	str retval = char_str(NULL,MAX_ALLOC_SIZE - sizeof(int) - 1);
	set_timeout(sc);
	retval->len = recv(sc->fd,retval->data,retval->len,0);
	if (retval->len < 0) retval->len = 0;
	return retval;
}

int
send_contents(Socket sc, Buffer buf, int chunked)
{
	int total = 0;
	if (!sc || !buf) return 0;
	if (buf->next) total = send_contents(sc,buf->next,chunked);
	total += (chunked ? write_chunked_socket(sc,buf->data,buf->length) : write_socket(sc,buf->data,buf->length));
	return total;
}

int
send_raw_contents(Socket sc, File fc, int off)
{
	if (!sc || !fc) return 0;
	return write_chunked_socket(sc,fc->data+off,min(fc->st.st_size-off,MAX_WRITE_SIZE));
}

