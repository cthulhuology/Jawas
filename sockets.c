// Sockets.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "log.h"
#include "sockets.h"
#include "timers.h"
#include "hostnames.h"
#include "client.h"
#include "server.h"
#include "tls.h"

#define STREAM 1
#define PACKET 0

void
signal_handler(int sig)
{
	if (client.response->socket) client.response->socket->closed = 1;
}

void
socket_signal_handlers()
{
	signal(SIGPIPE,signal_handler);
}

int
nonblock(reg fd)
{
	int flags = fcntl(fd,F_GETFL,0);
	return fcntl(fd,F_SETFL,flags | O_NONBLOCK);
}

int
keepalive(reg fd)
{
	int value = 1 ;
	return setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&value,sizeof(int));
}

int
nodelay(reg fd)
{
	int value = 1;
	return setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&value,sizeof(int));
}

int
socket_timeout(reg fd, size_t seconds)
{
	timeout(seconds,0);
	return setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&Timeout,sizeof(Timeout))
		&& setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&Timeout,sizeof(Timeout));
}

int
new_socket(int stream)
{
	int one = 1;
	reg fd = socket(AF_INET,stream ? SOCK_STREAM : SOCK_DGRAM,0);
	if (0 > fd) return 0;
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));
	return fd;
}

int
bind_socket(reg fd, IPAddress addr, int port)
{
	attachTo(addr,port);
	if (bind(fd,(struct sockaddr*)&Address,sizeof(Address))) {
		error("Failed to bind to port %i\n",port);	
		close(fd);
		return 0;
	}
	return fd;
}

int
listen_socket(reg fd) {
	if (listen(fd,128)) { // See man page on listen
		close(fd);
		return 0;
	}
	return fd;
}

int
open_socket(int  port)
{
	reg fd = new_socket(STREAM);
	if (!bind_socket(fd,(IPAddress)0,port)) error("Failed to bind to port %i",port);
	if (!listen_socket(fd)) error("Failed to listen to port %i",port);
	return fd;
}

Socket
resume_socket(Socket sc)
{
	sc->host = NULL;
	sc->buf = NULL;
	sc->closed = 0;
	return sc;
}

Socket
create_socket(reg fd, TLSInfo tls)
{
	Socket retval = (Socket)reserve(sizeof(struct socket_cache_struct));
	retval->fd = fd;
	retval->tls= (tls ? open_tls(tls,fd) : NULL);
	retval->peer = Address.sin_addr.s_addr;
	retval->port = Address.sin_port;
	return resume_socket(retval);
}

Socket
accept_socket(reg fd, TLSInfo tls)
{
	Address_len = sizeof(Address);
	reg sock = accept(fd,(struct sockaddr*)&Address,(socklen_t*)&Address_len);
	if (sock < 1) {
		error("[JAWAS] failed to accept socket");
		return NULL;
	}
	nonblock(sock);
	keepalive(sock);
	socket_timeout(sock,SOCKET_CONNECT_TIMEOUT);
	Socket retval = create_socket(sock,tls);
	while (tls && 0 < accept_tls(retval->tls));
	return retval;
}

Socket
connect_socket(str host, int port, int ssl)
{
	Socket retval;
	int i;
	IPAddress* ipaddrs = lookup(host);
	int sock = new_socket(1);
	if (!sock) {
		error("[JAWAS] failed to create socket");
		return NULL;
	}
	int connected = 0; 
	for (i = 0; ipaddrs[i]; ++i) {
		debug("Connecting to socket %s",ipaddress(ipaddrs[i],port));
		attachTo(ipaddrs[i],port);
		timeout(SOCKET_CONNECT_TIMEOUT,0);
		timer();
		connected = 1;
		if (connect(sock,(struct sockaddr*)&Address,Address_len)) {
			if (client.alarm) {
				error("[JAWAS] socket conect timeout!");
				client.alarm = 0;
			}
			perror("connect");
			error("[JAWAS] failed to connect to %c:%i",host,port);
			connected = 0;
			continue;
		}
		client.alarm = 0;
		timeout(0,0);
		timer();
		break;
	}
	if (!connected) return NULL;
	keepalive(sock);
	socket_timeout(sock,SOCKET_CONNECT_TIMEOUT);
	retval = create_socket(sock,ssl ? server.tls_client :NULL);
	retval->host = _("%c",host);
	retval->peer = Address.sin_addr.s_addr;
	retval->port = port;
	if (ssl && (connect_tls(retval->tls) ||check_tls(retval->tls))) {
			error("[SSL] Failed to make secure connection");
			return NULL;
	}
	return retval;
}

void
close_socket(Socket sc)
{
	if (!sc) return;
	if (sc->tls) close_tls(sc->tls);
	close(sc->fd);
}

str
read_socket(Socket sc)
{
	int delta;
	for (sc->buf = sc->buf ? clone(sc->buf) : blank(0); 
		(delta = sc->tls ?
			read_tls(sc->tls,sc->buf->data + sc->buf->length,Max_Buffer_Size) :
			read(sc->fd,sc->buf->data + sc->buf->length,Max_Buffer_Size)); 
		sc->buf->length += advance(delta)) {
		fprintf(stderr,"Read %i bytes at %p\n",delta,sc->buf);
		if (delta == 0) {
			fprintf(stderr,"READ ZERO!!!\n");
			sc->closed = 1;
			return sc->buf;
		}
		if (delta == -1 ) {
			if (errno == EAGAIN) {
				fprintf(stderr,"Returning %d bytes [%s]\n",len(sc->buf),sc->buf->data);
				return sc->buf;
			}
			error("ERROR %i occured", errno);
			return NULL;	
		}
	}
	return sc->buf;
}

int
write_to_socket(Socket sc,char* data, int length)
{
	if (sc->closed) return 0;
	int retval = sc->tls ? write_tls(sc->tls,data,length) : write(sc->fd,data,length);
	if (retval < 0) {
		sc->closed = 1;
		return 0;
	}
	return retval;
}

int
write_socket(Socket sc, str buf)
{
	if (! sc || !buf) return 0;
	return write_to_socket(sc,buf->data,buf->length);
}

int
write_chunk(Socket sc, char* data, int length)
{
	int retval = 0;
	str header = _("%h\r\n",length);
	write_to_socket(sc,header->data,header->length);
	if (data) retval = write_to_socket(sc,data,length);
	write_to_socket(sc,"\r\n",2);
	return retval;
}

int
write_chunked_socket(Socket sc, str buf)
{
	if (!sc) return 0;
	int i, retval = 0;
	for (i = 0; i < buf->length; i += MAX_WRITE_SIZE)
		retval += write_chunk(sc,buf->data + i,min(MAX_WRITE_SIZE,buf->length - i));
	return retval;
}

int
send_contents(Socket sc, str buf, int chunked)
{
	if (!sc || !buf) return 0;
	return chunked ? write_chunked_socket(sc,buf) : write_socket(sc,buf);
}

int
write_raw_socket(Socket sc, char* data, int length)
{
	int retval = write_to_socket(sc,data,length);
	if (retval < 0) {
		retval = 0;
		sc->closed = 1;
	}
	return retval;
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
send_raw_contents(Socket sc, File fc, int off, int chunked)
{
	if (!sc || !fc) return 0;
	return chunked ? 
		write_raw_chunked_socket(sc,fc->data+off,min(fc->st.st_size-off,MAX_WRITE_SIZE)) :
		write_raw_socket(sc,fc->data +off,min(fc->st.st_size-off,MAX_WRITE_SIZE));
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

str
socket_peer(Socket sc)
{
	return ipaddress(sc->peer,sc->port);
}

void
socket_notice(Socket sc, char* msg)
{
	notice("Socket <%p> %s %c\n", sc, socket_peer(sc), msg);
}

void
socket_attach(Socket sc, IPAddress peer, int port)
{
	sc->peer = peer;
	sc->port = port;
}

size_t
socket_send(Socket sc, str msg)
{
	attachTo(sc->peer,sc->port);
	return sendto(sc->fd,msg->data,msg->length,0,(struct sockaddr *)&Address,sizeof(Address));
}

str
socket_recv(Socket sc)
{
	attachTo(sc->peer,sc->port);
	Address_len = sizeof(Address);
	size_t msg_size;
	recvfrom(sc->fd,&msg_size,sizeof(size_t),0, (struct sockaddr*)&Address, (socklen_t *)&Address_len);
	str retval = blank(msg_size);
	retval->length = recvfrom(sc->fd, retval->data, retval->length, 0, (struct sockaddr *)&Address, (socklen_t *)&Address_len);
	return retval;
}
