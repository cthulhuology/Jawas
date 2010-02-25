// script.c
//
// Copyright (C) 2009 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "server.h"
#include "image.h"
#include "server.h"
#include "dates.h"
#include "status.h"
#include "gzip.h"

int
script_handler(File fc)
{
	int total = 0;
	cache_control(server.response->headers,"max-age=86400, public");
	date_field(server.response->headers,Date(time(NULL))->data);
	expires(server.response->headers,Expires()->data);
//	str zipped = shrink(fc);
//	if (zipped) append_header(server.response->headers,Str("Content-Encoding"),Str("gzip"));
//	debug("RESP:\n%s",print_headers(NULL,server.response->headers));
	send_status(server.response->socket,200);
	send_headers(server.response->socket,server.response->headers);
	while (!server.response->socket->closed && total < fc->st.st_size)
		total += send_raw_contents(server.response->socket,fc,total,1);
//		total += zipped ? send_contents(server.response->socket,zipped,1):
	return 200;
}
