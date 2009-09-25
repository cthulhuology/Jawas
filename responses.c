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
	return write_socket(sc,status_line(code));
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
		str hed = from(resp->contents,0,resp->body);
		str con = dechunk(resp->contents);
		resp->contents = append(hed,con);
	}
	return resp;
}


Response
process_response(Response resp)
{
	resp->contents = read_socket(resp->sc);
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
		(len(resp->contents) - resp->body) >= inbound_content_length(resp->contents,resp->headers);
	return resp->done ? dechunk_response(resp) : resp;
}

int
send_response(Response resp)
{
	if (resp->length < 0) {
		resp->length = outbound_content_length(resp->contents,resp->raw_contents);
		if (!Resp->headers) {
			error("Missing headers, adding new ones");
			Resp->headers = new_headers();
		}
		connection(Resp->headers,"close");
		transfer_encoding(Resp->headers,"chunked");
		server(resp->headers,server_name);
		send_status(resp->sc,resp->status);
		send_headers(resp->sc,resp->headers);
		return resp->contents || resp->raw_contents;
	}
	resp->written += resp->contents ?
		send_contents(resp->sc,resp->contents,1):
		send_raw_contents(resp->sc,resp->raw_contents,resp->written,1);
	fprintf(stderr,"written %d length %d\n",resp->written,resp->length);
	if (resp->written >= resp->length)
		write_chunk(resp->sc,NULL,0);
	return resp->written < resp->length;
}

