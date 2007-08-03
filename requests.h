// requests.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "headers.h"
#include "buffers.h"
#include "sockets.h"

typedef struct request_struct* Request;
struct request_struct {
	Socket sc;
	Headers headers;
	Headers query_vars;
	Buffer contents;	
	str host;
	str path;
	int body;
	int done;
};

Request open_request(Socket sc);
Request read_request(Request req);
int request_content_length(Request req);
void close_request(Request req);

str parse_method();
str parse_host();
str parse_path();

#endif
