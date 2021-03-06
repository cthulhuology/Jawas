// image.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "client.h"
#include "sockets.h"
#include "image.h"
#include "headers.h"
#include "client.h"
#include "dates.h"
#include "status.h"

int
img_handler(File fc)
{
	int total = 0;
	cache_control(client.response->headers,"max-age=86400, public");
	date_field(client.response->headers,Date(time(NULL))->data);
	expires(client.response->headers,Expires()->data);
	send_status(client.response->socket,200);
	send_headers(client.response->socket,client.response->headers);
	str range = find_header(client.response->request->headers,_("Range"));
	if (range) {
		debug("Ranges: %s",range);
		int start = find(range,0,"0123456789",10);
		int dash = find(range,start,"-",1);
		debug("Start range: %s",ref(range->data+start,dash-start));	
		debug("End range: %s",ref(range->data+dash+1,len(range)-dash));	
	}
	while (! client.response->socket->closed && total < fc->st.st_size)
		total += send_raw_contents(client.response->socket,fc,total,1);
	return 200;
}
