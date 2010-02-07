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
	cache_control(Resp->headers,"max-age=86400, public");
	date_field(Resp->headers,Date(time(NULL))->data);
	expires(Resp->headers,Expires()->data);
//	str zipped = shrink(fc);
//	if (zipped) append_header(Resp->headers,Str("Content-Encoding"),Str("gzip"));
//	debug("RESP:\n%s",print_headers(NULL,Resp->headers));
	send_status(Resp->sc,200);
	send_headers(Resp->sc,Resp->headers);
	while (!Resp->sc->closed && total < fc->st.st_size)
		total += send_raw_contents(Resp->sc,fc,total,1);
//		total += zipped ? send_contents(Resp->sc,zipped,1):
	return 200;
}
