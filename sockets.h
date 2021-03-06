// Sockets.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __SOCKETS_H__
#define __SOCKETS_H__

#include "str.h"
#include "tls.h"
#include "files.h"
#include "hostnames.h"

typedef struct socket_cache_struct* Socket;
struct socket_cache_struct {
	reg fd;
	TLSSocket tls;
	str buf;
	str host;
	unsigned int peer;
	int port;
	int closed;
};

extern Socket sockets;

void socket_signal_handlers();
int nonblock(reg);
int nodelay(reg);

int new_socket(int stream);
int open_socket(int port);
Socket create_socket(reg fd, TLSInfo tls);
Socket accept_socket(reg fd, TLSInfo tls);
Socket connect_socket(str host, int port, int ssl);
Socket resume_socket(Socket sc);
int send_contents(Socket sc, str buf, int chunked);
int send_raw_contents(Socket sc, File fc, int off, int chunked);
str read_socket(Socket sc);
int write_to_socket(Socket sc, char* buf, int length);
int write_socket(Socket sc, str buf);
int write_chunked_socket(Socket sc, str buf);
int write_chunk(Socket sc, char* data, int length);
int closed_socket(Socket sc, char* msg);
void socket_notice(Socket sc, char* msg);
str socket_peer(Socket sc);
int socket_timeout(reg fd, size_t seconds);

void socket_attach(Socket sc, IPAddress peer, int port);
size_t socket_send(Socket sc, str msg);
str socket_recv(Socket sc);

void close_socket(Socket sc);
void close_sockets();

#endif
