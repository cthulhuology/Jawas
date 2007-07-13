// responses.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "defines.h"
#include "headers.h"
#include "status.h"
#include "responses.h"
#include "sockets.h"
#include "uri.h"

int
dup_write(Socket sc, char* buffer, size_t len)
{
	write(1,buffer,len);
	return write_socket(sc,buffer,len);
}

int
calculate_content_length(Buffer buf)
{
	int total = 0;
	if (! buf) return 0;
	for (total = 0; buf; buf = buf->next) total += buf->length;
	return total;
}

int
send_status(Socket sc, int code)
{
	int total = 0;
	char* status = status_line(code);
	fprintf(stderr,"Status line length %d\n",strlen(status));
	total += dup_write(sc,status,strlen(status));
	free(status);
	return total;
}

int
send_headers(Socket sc, Headers headers)
{
	int i;
	int total = 0;
	if (!headers) return total;
	for (i = 0; headers[i].key; ++i) {
		total += dup_write(sc,headers[i].key->data,headers[i].key->length);
		total += dup_write(sc,":",1);
		total += dup_write(sc,headers[i].value->data,headers[i].value->length);
		total += dup_write(sc,"\r\n",2);
	}
	total += dup_write(sc,"\r\n",2);
	return total;
}


int
send_contents(Socket sc, Buffer buf)
{
	int total = 0;
	if (!buf) return 0;
	if (buf->next) total = send_contents(sc,buf->next);
	total += dup_write(sc,buf->data,buf->length);
	return total;
}

static char* server_name = SERVER_VERSION;

Response 
process_request(Request req)
{
	Response resp = (Response)malloc(sizeof(struct response_struct));
	resp->sc = req->sc;
	resp->req = req;
	resp->headers = new_headers();
	resp->status = 200;
	resp->contents = NULL;
	return resp;
}

int
send_response(Response resp)
{
	int total = 0;
	char* buffer = malloc(NUM_BUFFER_SIZE);

	fprintf(stderr,"send_response Response %p, headers %p\n",resp,resp->headers);
	memset(buffer,0,NUM_BUFFER_SIZE);
	snprintf(buffer,NUM_BUFFER_SIZE,"%d",calculate_content_length(resp->contents));
	content_length(resp->headers,buffer);
	server(resp->headers,server_name);
	dump_headers(resp->headers);
	total = 0;
	total += send_status(resp->sc,resp->status);
	total += send_headers(resp->sc,resp->headers);
	total += send_contents(resp->sc,resp->contents);
	fprintf(stderr,"Wrote %d bytes\n",total);
	free(buffer);
	return total;
}

void
close_response(Response resp)
{
	Buffer buf;
	if (! resp) return;
	fprintf(stderr,"Freeing response %p\n",resp);
	close_request(resp->req);
	free_headers(resp->headers);
	for (buf = resp->contents; buf; buf = free_buffer(buf));
	free(resp);
}


