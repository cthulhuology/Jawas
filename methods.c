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

Headers
parse_json(Headers hd, str buf, int pos)
{
	debug("Found json [%s]",ref(buf->data+pos,len(buf)-pos));
	return append_header(hd,$("json"),ref(buf->data + pos,len(buf)-pos));
}

int
get_method()
{
	File fc = NULL;
	str filename;
	if (! server.request || !server.request->host || !server.request->path) return error_handler(404);
	notice("GET %s%s from %s",server.request->host,server.request->path,socket_peer(server.request->socket));
	filename = deauth_path(file_path(server.request->host,server.request->path));

	fc = is_directory(filename) ?
		load(get_index(filename)) :
		load(filename);
	return mimetype_handler(fc);
}

int
post_method()
{
	str enctype = find_header(server.request->headers,$("Content-Type"));
	notice("POST %s%s %s from %s",server.request->host,server.request->path,enctype, socket_peer(server.request->socket));
	server.request->query_vars = enctype && ncmp(enctype,$("multipart/form-data"),19) ?
			parse_multipart_body(server.request->query_vars,enctype) :
		enctype && ncmp(enctype,$("text/json"),9) ?
			parse_json(server.request->query_vars,server.request->contents,server.request->body):
		parse_uri_encoded(server.request->query_vars,server.request->contents,server.request->body);
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
