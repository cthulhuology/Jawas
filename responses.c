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
	debug("Sending status [%s]",status_line(code));
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
		// debug("HEADERS [%s]",hed);
		str con = dechunk(resp->contents);
		// debug("DECHUNKED CONTENTS [%s]",con);
		resp->contents = append(hed,con);
		// debug("RESP CONTNETS [%s]",resp->contents);
	}
	return resp;
}


Response
process_response(Response resp)
{
	debug("process_response");
	resp->contents = read_socket(resp->sc);
	debug("resp->contents [%s]", resp->contents);
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
	debug("response length %i",resp->length);
	debug("Response headers are: [%s]",print_headers(NULL,resp->headers));
	debug("Response contents are: %i [%s]",len(resp->contents),resp->contents);
	if (resp->length < 0) {
		resp->length = outbound_content_length(resp->contents,resp->raw_contents);
		debug("Outbound content length %i",resp->length);
		server(resp->headers,server_name);
		debug("Set server name [%c]",server_name);	
		send_status(resp->sc,resp->status);
		debug("Sent status [%i]",resp->status);
		send_headers(resp->sc,resp->headers);
		debug("Resp  headers [%s]",print_headers(NULL,resp->headers));
		debug("Response contents [%s]", resp->contents);
		return resp->contents || resp->raw_contents;
	}
	debug("Writing response contents [%s]",resp->contents);
	resp->written += resp->contents ?
		send_contents(resp->sc,resp->contents,1):
		send_raw_contents(resp->sc,resp->raw_contents,resp->written);
	debug("Wrote %i",resp->written);
	if (resp->written >= resp->length) {
		debug("Writing null chunk EOF");
		write_chunk(resp->sc,NULL,0);
	}
	debug("Done? %c", (resp->written >= resp->length) ? "yes" : "now");
	return resp->written < resp->length;
}

