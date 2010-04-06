// script.c
//
// Copyright (C) 2009 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "client.h"
#include "image.h"
#include "dates.h"
#include "status.h"

int
script_handler(File fc)
{
	int total = 0;
//	cache_control(client.response->headers,"max-age=86400, public");
//	date_field(client.response->headers,Date(time(NULL))->data);
//	expires(client.response->headers,Expires()->data);
	send_status(client.response->socket,200);
	send_headers(client.response->socket,client.response->headers);
	while (!client.response->socket->closed && total < fc->st.st_size)
		total += send_raw_contents(client.response->socket,fc,total,1);
	return 200;
}
