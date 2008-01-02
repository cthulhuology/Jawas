// methods.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "log.h"
#include "status.h"
#include "uri.h"
#include "index.h"
#include "server.h"
#include "methods.h"
#include "mime.h"
#include "forms.h"

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
	File fc = NULL;
	str filename;
	debug("GET %s%s",Req->host,Req->path);
	filename = file_path(Req->host,Req->path);
	fc = is_directory(filename) ? 
		load(get_index(filename)) :
		load(filename);
	return mimetype_handler(fc);
}

int
post_method()
{
	str enctype = find_header(Req->headers,Str("Content-Type"));
	debug("POST %s%s %s",Req->host,Req->path,enctype);
//	debug("Enctype: %s (%p,%i)", enctype, enctype->data,enctype->len);
//	debug("multipart? %c", (ncmp(enctype,Str("multipart/form-data"),19) ? "yes" : "no"));
	Req->query_vars = enctype && ncmp(enctype,Str("multipart/form-data"),19) ?
		parse_multipart_body(Req->query_vars,enctype) :
		parse_uri_encoded(Req->query_vars,Req->contents,Req->body);
	return get_method();
}

NOT_IMPLEMENTED(head_method)
NOT_IMPLEMENTED(put_method)
NOT_IMPLEMENTED(delete_method)
NOT_IMPLEMENTED(options_method)
NOT_IMPLEMENTED(trace_method)

int
dispatch_method(str method)
{
	int i;
//	debug("Method is %s",method);
	for (i = 0; gdispatch[i].name; ++i ) 
		if (! strncasecmp(gdispatch[i].name,method->data,gdispatch[i].len)) 
			return gdispatch[i].handler();
	error("Bad request: %s",method);
	return error_handler(400);
}
