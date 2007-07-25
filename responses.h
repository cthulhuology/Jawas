// responses.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include "headers.h"
#include "buffers.h"
#include "requests.h"
#include "sockets.h"
#include "files.h"

typedef struct response_struct* Response;
struct response_struct {
	Socket sc;
	Request req;
	Headers headers;
	Buffer contents;
	File raw_contents;
	int status;
	int length;
	int written;
	int done;
};

Response process_request(Request req);
int send_response(Response resp);
void close_response(Response resp);

#endif
