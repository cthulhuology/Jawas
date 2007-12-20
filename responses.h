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

struct response_struct {
	Socket sc;
	Request req;
	Headers headers;
	Buffer contents;
	File raw_contents;
	int status;
	int length;
	int written;
	int body;
	int done;
};

Response new_response(Request req);
int send_response(Response resp);
void close_response(Response resp);
Response process_response(Response resp);

#endif
