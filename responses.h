// responses.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include "str.h"
#include "headers.h"
#include "requests.h"
#include "sockets.h"
#include "files.h"

struct response_struct {
	Socket socket;
	Request request;
	Headers headers;
	str contents;
	File raw_contents;
	int status;
	int length;
	int written;
	int body;
	int done;
};

extern Response responses;

Response new_response(Request req);
int begin_response();
int end_response();
void close_response();
int process_response();

#endif
