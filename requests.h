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
	Socket socket;
	Response response;
	Headers headers;
	Headers query_vars;
	File raw_contents;
	Usage usage;
	str contents;	
	str cb;
	str method;
	str host;
	int port;
	str path;
	int body;
	int done;
	int written;
	int length;
	int retries;
	int ssl;
};

typedef struct request_info_struct* RequestInfo;
struct request_info_struct {
	RequestInfo next;
	str host;
	str path;
	int time;
	int hits;	
};

extern Request requests;

Request new_request(str method, str host, str path);
Request open_request(Socket sc);
void request_port(Request req, int port);
void request_ssl(Request req);
void request_headers(Request req, str key, str value);
void request_data(Request req, str text);
void request_file(Request req, File fc);
void request_callback(Request req, Response resp, str cb);
int send_request(Request req);

int process_request();

int request_content_length();

str parse_method();
str parse_host(Request req, str host);
str parse_path();

#endif
