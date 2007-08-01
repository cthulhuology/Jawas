// methods.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "status.h"
#include "uri.h"
#include "index.h"
#include "server.h"
#include "methods.h"

MethodDispatch gdispatch[] = {
	{ 3, "GET", get_method },
	{ 4, "POST", post_method },
	{ 4, "HEAD", head_method },
	{ 7, "OPTIONS", options_method },
	{ 3, "PUT", put_method },
	{ 6, "DELETE", delete_method },
	{ 5, "TRACE", trace_method },
	{ 0, NULL, NULL }
};

int
get_method(Server srv, Response resp)
{
	File fc;
	char* filename;
	filename = request_path(resp->req);
	fc = is_directory(filename) ? 
		load(srv,get_index(filename)) :
		load(srv,filename);
	return mimetype_handler(srv,fc,resp);
}

int
head_method(Server srv, Response resp)
{
	return error_handler(srv,405,resp);
}

int
post_method(Server srv, Response resp)
{
	parse_post_request(resp->req);
	return get_method(srv,resp);
}

int
put_method(Server srv, Response resp)
{
	return error_handler(srv,405,resp);
}

int
delete_method(Server srv, Response resp)
{
	return error_handler(srv,405,resp);
}

int
options_method(Server srv, Response resp)
{
	return error_handler(srv,405,resp);
}

int
trace_method(Server srv, Response resp)
{
	return error_handler(srv,405,resp);
}

int
dispatch_method(Server srv, char* method, Response resp)
{
	int i;
	debug("Dispatch method: %s",method);
	for (i = 0; gdispatch[i].name; ++i ) 
		if (! strncasecmp(gdispatch[i].name,method,gdispatch[i].len)) 
			return gdispatch[i].handler(srv,resp);
	error("Bad request: %s",method);
	return error_handler(srv,400,resp);
}

