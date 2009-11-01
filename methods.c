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
#include "json.h"

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
	if (! Req || !Req->host || !Req->path) return error_handler(404);
	notice("GET %s%s from %s",Req->host,Req->path,socket_peer(Req->sc));
	filename = deauth_path(file_path(Req->host,Req->path));

	fc = is_directory(filename) ?
		load(get_index(filename)) :
		load(filename);
	return mimetype_handler(fc);
}

int
post_method()
{
	str enctype = find_header(Req->headers,Str("Content-Type"));
	notice("POST %s%s %s from %s",Req->host,Req->path,enctype, socket_peer(Req->sc));
	Req->query_vars = enctype && ncmp(enctype,Str("multipart/form-data"),19) ?
			parse_multipart_body(Req->query_vars,enctype) :
		enctype && ncmp(enctype,Str("text/json"),9) ?
			parse_json(Req->query_vars,Req->contents,Req->body):
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
	for (i = 0; gdispatch[i].name; ++i ) 
		if (gdispatch[i].len == len(method) && !strncasecmp(gdispatch[i].name,method->data,gdispatch[i].len)) 
			return gdispatch[i].handler();
	error("Bad request: %s",method);
	return error_handler(400);
}
