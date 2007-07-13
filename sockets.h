// Sockets.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __SOCKETS_H__
#define __SOCKETS_H__

#include "buffers.h"
#include "tls.h"

typedef struct socket_cache_struct* Socket;
struct socket_cache_struct {
	Socket next;
	Buffer buf;
	TLSSocket tls;
	int fd;
	unsigned int peer;
	int port;
};

void socket_signal_handlers();

int open_socket(int port);
Socket accept_socket(Socket sc, int fd, TLSInfo tls);

Buffer read_socket(Socket sc);
int write_socket(Socket sc, char* src, int len);
Socket close_socket(Socket sc);

#endif
