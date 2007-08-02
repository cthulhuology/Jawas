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
get_method()
{
	File fc;
	str filename;
	filename = file_path(Req->host,Req->path);
	fc = is_directory(filename) ? 
		load(get_index(filename)) :
		load(filename);
	return mimetype_handler(fc);
}

int
head_method()
{
	return error_handler(405);
}

int
post_method()
{
	parse_post_request(Req);
	return get_method();
}

int
put_method()
{
	return error_handler(405);
}

int
delete_method()
{
	return error_handler(405);
}

int
options_method()
{
	return error_handler(405);
}

int
trace_method()
{
	return error_handler(405);
}

int
dispatch_method(str method)
{
	int i;
	for (i = 0; gdispatch[i].name; ++i ) 
		if (! strncasecmp(gdispatch[i].name,method->data,gdispatch[i].len)) 
			return gdispatch[i].handler();
	error("Bad request: %s",method);
	return error_handler(400);
}

