// responses.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "headers.h"
#include "status.h"
#include "responses.h"
#include "sockets.h"
#include "uri.h"

int
calculate_content_length(Buffer buf, File fc)
{
	int total = 0;
	if (! buf) return (fc ? fc->st.st_size : 0);
	for (total = 0; buf; buf = buf->next) total += buf->length;
	return total;
}

int
send_status(Socket sc, int code)
{
	int total = 0;
	char* status = status_line(code);
	total += write_socket(sc,status,strlen(status));
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
		total += write_socket(sc,headers[i].key->data,headers[i].key->length);
		total += write_socket(sc,":",1);
		total += write_socket(sc,headers[i].value->data,headers[i].value->length);
		total += write_socket(sc,"\r\n",2);
	}
	total += write_socket(sc,"\r\n",2);
	return total;
}


int
send_contents(Socket sc, Buffer buf)
{
	int total = 0;
	if (!sc || !buf) return 0;
	if (buf->next) total = send_contents(sc,buf->next);
	total += write_socket(sc,buf->data,buf->length);
	return total;
}

int
send_raw_contents(Socket sc, File fc)
{
	int total = 0;
	if (!sc || !fc) return 0;
	while (total < fc->st.st_size)
		total += write_socket(sc,fc->data+total,fc->st.st_size-total);
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
	resp->raw_contents = NULL;
	return resp;
}

int
send_response(Response resp)
{
	int total = 0;
	char* buffer = malloc(NUM_BUFFER_SIZE);

	memset(buffer,0,NUM_BUFFER_SIZE);
	snprintf(buffer,NUM_BUFFER_SIZE,"%d",calculate_content_length(resp->contents,resp->raw_contents));
	content_length(resp->headers,buffer);
	server(resp->headers,server_name);
	total = 0;
	total += send_status(resp->sc,resp->status);
	total += send_headers(resp->sc,resp->headers);
	total += resp->contents ?
		send_contents(resp->sc,resp->contents):
		send_raw_contents(resp->sc,resp->raw_contents);
	free(buffer);
	debug("Sent %i bytes",total);
	return total;
}

void
close_response(Response resp)
{
	Buffer buf;
	if (! resp) return;
	debug("Freeing response %i\n",resp);
	close_request(resp->req);
	free_headers(resp->headers);
	for (buf = resp->contents; buf; buf = free_buffer(buf));
	free(resp);
}


