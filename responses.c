// responses.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
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
	return total;
}

int
send_headers(Socket sc, Headers headers)
{
	int i;
	int total = 0;
	if (!headers) return total;
	for (i = 0; headers[i].key; ++i) {
		total += write_socket(sc,headers[i].key->data,headers[i].key->len);
		total += write_socket(sc,": ",2);
		total += write_socket(sc,headers[i].value->data,headers[i].value->len);
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
	total += write_chunked_socket(sc,buf->data,buf->length);
	return total;
}

int
send_raw_contents(Socket sc, File fc, int off)
{
	if (!sc || !fc) return 0;
	return write_chunked_socket(sc,fc->data+off,min(fc->st.st_size-off,MAX_WRITE_SIZE));
}

static char* server_name = SERVER_VERSION;

Response 
process_request(Request req)
{
	Response resp = (Response)salloc(sizeof(struct response_struct));
	resp->sc = req->sc;
	resp->req = req;
	resp->headers = new_headers();
	resp->status = 200;
	resp->contents = NULL;
	resp->raw_contents = NULL;
	resp->length = -1;
	resp->written = 0;
	resp->done = 0;
	return resp;
}

int
send_response(Response resp)
{
	if (resp->length < 0) {
		resp->length = calculate_content_length(resp->contents,resp->raw_contents);
	//	str len = Str("%i",resp->length);
	//	content_length(resp->headers,len->data);
		server(resp->headers,server_name);
		send_status(resp->sc,resp->status);
		send_headers(resp->sc,resp->headers);
		if (resp->contents || resp->raw_contents)
			return 1;
		return 0;
	}
	resp->written += resp->contents ?
		send_contents(resp->sc,resp->contents):
		send_raw_contents(resp->sc,resp->raw_contents,resp->written);
	if (resp->written >= resp->length)
		write_chunked_socket(resp->sc,NULL,0);
	return resp->written < resp->length;
}

void
close_response(Response resp)
{
	Buffer buf;
	if (! resp) return;
	close_request(resp->req);
	free_headers(resp->headers);
	for (buf = resp->contents; buf; buf = free_buffer(buf));
}

