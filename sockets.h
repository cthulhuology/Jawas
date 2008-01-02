// Sockets.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __SOCKETS_H__
#define __SOCKETS_H__

#include "alloc.h"
#include "str.h"
#include "tls.h"
#include "files.h"

typedef struct socket_cache_struct* Socket;
struct socket_cache_struct {
	Socket next;
	Scratch scratch;	
	str buf;
	TLSSocket tls;
	int fd;
	unsigned int peer;
	int port;
	str host;
	int closed;
};

typedef struct socket_info_struct SocketInfo;
struct socket_info_struct {
	size_t max;
	size_t current;
	size_t total;
};

extern SocketInfo gsci;

void socket_signal_handlers();
int nonblock(int fd);

int open_socket(int port);
Socket accept_socket(Socket sc, int fd, TLSInfo tls);
Socket connect_socket(char* host, int port);
int send_contents(Socket sc, str buf, int chunked);
int send_raw_contents(Socket sc, File fc, int off);
str read_socket(Socket sc);
int write_socket(Socket sc, str buf);
int write_chunked_socket(Socket sc, str buf);
int write_chunk(Socket sc, char* data, int length);
Socket reset_socket(Socket sc);
Socket close_socket(Socket sc);
int closed_socket(Socket sc, char* msg);
void socket_notice(Socket sc, char* msg);

#endif
