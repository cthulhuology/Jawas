// responses.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "log.h"
#include "headers.h"
#include "status.h"
#include "responses.h"
#include "sockets.h"
#include "uri.h"
#include "transfer.h"

Response responses;

Response
new_response(Request req)
{
	Response resp = (Response)reserve(sizeof(struct response_struct));
	resp->request = req;
	resp->socket = req->socket;
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

void
dechunk_response()
{
	if (is_chunked(server.response->headers)) {
		str hed = ref(server.response->contents->data,server.response->body);
		str con = dechunk(server.response->contents);
		server.response->contents = append(hed,con);
	}
}

int
process_response()
{
	server.response->contents = read_socket(server.response->socket);
	if (! server.response->contents) {
		error("No response contents on Response <%p>\n",server.response);
		return -1;
	}
	if (! server.response->body) {
		server.response->headers = parse_headers(server.response->contents,&server.response->body);
		if (! server.response->headers) {
			error("No response headers on Response <%p>\n",server.response);
			return -1;
		}
	}
	server.response->done = server.response->body &&
		(len(server.response->contents) - server.response->body) >= inbound_content_length(server.response->contents,server.response->headers);
	if (server.response->done) dechunk_response();
	return 0;
}

int
begin_response()
{
	if (!server.response->headers) {
		error("Missing headers, adding new ones");
		server.response->headers = new_headers();
	}
	connection(server.response->headers,"close");
	transfer_encoding(server.response->headers,"chunked");
	server_name(server.response->headers,SERVER_VERSION);

	return server.response->contents || server.response->raw_contents;
}

int
end_response()
{
	write_chunk(server.response->socket,NULL,0);
	return 0;
}

