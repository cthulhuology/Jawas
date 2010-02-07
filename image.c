// image.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "server.h"
#include "sockets.h"
#include "image.h"
#include "headers.h"
#include "server.h"
#include "dates.h"
#include "status.h"

int
img_handler(File fc)
{
	int total = 0;
	cache_control(Resp->headers,"max-age=86400, public");
	date_field(Resp->headers,Date(time(NULL))->data);
	expires(Resp->headers,Expires()->data);
	send_status(Resp->sc,200);
	send_headers(Resp->sc,Resp->headers);
	str range = find_header(Resp->req->headers,Str("Range"));
	if (range) {
		debug("Ranges: %s",range);
	}
	while (! Resp->sc->closed && total < fc->st.st_size)
		total += send_raw_contents(Req->sc,fc,total,1);
	return 200;
}
