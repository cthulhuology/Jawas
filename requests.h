// requests.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "str.h"
#include "headers.h"
#include "sockets.h"
#include "usage.h"

typedef struct response_struct* Response;
typedef struct request_struct* Request;
struct request_struct {
	Socket sc;
	Headers headers;
	Headers query_vars;
	str contents;	
	File raw_contents;
	Usage usage;
	Response resp;
	str cb;
	str method;
	str host;
	int port;
	str path;
	int body;
	int done;
	int written;
	int length;
};

typedef struct request_info_struct* RequestInfo;
struct request_info_struct {
	RequestInfo next;
	str host;
	str path;
	int time;
	int hits;	
};

Request new_request(str method, str host, str path);
Request request_headers(Request req, str key, str value);

Request request_data(Request req, str text);
Request request_file(Request req, File fc);

Request open_request(Socket sc);
Request process_request(Request req);

int send_request(Request req);
int request_content_length(Request req);
void close_request(Request req);
void request_callback(Request req, Response resp, str cb);

str parse_method();
str parse_host();
str parse_path();

RequestInfo start_request(RequestInfo ri, Request req);
RequestInfo end_request(RequestInfo ri, Request req);

#endif
