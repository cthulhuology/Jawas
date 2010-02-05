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

int
script_handler(File fc)
{
	int total = 0;
	cache_control(Resp->headers,"max-age=86400, public");
	date_field(Resp->headers,Date(time(NULL))->data);
	expires(Resp->headers,Expires()->data);
	send_status(Resp->sc,200);
	send_headers(Resp->sc,Resp->headers);
	while (total < fc->st.st_size)
		total += send_raw_contents(Req->sc,fc,total,1);
	return 200;
}
