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
#include "transfer.h"

int
send_status(Socket sc, int code)
{
	int total = 0;
	char* status = status_line(code);
	total += write_socket(sc,status,strlen(status));
	return total;
}

static char* server_name = SERVER_VERSION;

Response
new_response(Request req)
{
	Response resp = (Response)salloc(sizeof(struct response_struct));
	resp->req = req;
	resp->sc = req->sc;
	resp->headers = new_headers();
	resp->status = 200;
	resp->contents = NULL;
	resp->raw_contents = NULL;
	resp->length = -1;
	resp->written = 0;
	resp->body = 0;
	resp->done = 0;
	return resp;
}

Response
dechunk_response(Response resp)
{
	if (is_chunked(resp->headers)) {
		Buffer hed = read_buffer(NULL,resp->contents,0,resp->body);
		Buffer con = dechunk_buffer(resp->contents);
		resp->contents = read_buffer(hed,con,0,length_buffer(con));
	}
	return resp;
}


Response
process_response(Response resp)
{
	debug("process_response");
	resp->contents = read_socket(resp->sc);
	debug("resp->contents %p", resp->contents);
	if (! resp->contents) {
		error("No response contents on Response <%p>\n",resp);
		return NULL;
	}
	if (! resp->body) {
		resp->headers = parse_headers(resp->contents,&resp->body);
		if (! resp->headers) {
			error("No response headers on Response <%p>\n",resp);
			return NULL;
		}
	}
	resp->done = resp->body &&
		(length_buffer(resp->contents) - resp->body) >= inbound_content_length(resp->contents,resp->headers);
	return resp->done ? dechunk_response(resp) : resp;
}

int
send_response(Response resp)
{
	if (resp->length < 0) {
		resp->length = outbound_content_length(resp->contents,resp->raw_contents);
		server(resp->headers,server_name);
		send_status(resp->sc,resp->status);
		send_headers(resp->sc,resp->headers);
		return resp->contents || resp->raw_contents;
	}
	resp->written += resp->contents ?
		send_contents(resp->sc,resp->contents,1):
		send_raw_contents(resp->sc,resp->raw_contents,resp->written);
	if (resp->written >= resp->length)
		write_chunked_socket(resp->sc,NULL,0);
	return resp->written < resp->length;
}

