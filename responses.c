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
#include "client.h"

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
	if (is_chunked(client.response->headers)) {
		str hed = ref(client.response->contents->data,client.response->body);
		str con = dechunk(client.response->contents);
		client.response->contents = append(hed,con);
	}
}

int
process_response()
{
	client.response->contents = read_socket(client.response->socket);
	if (! client.response->contents) {
		error("No response contents on Response <%p>\n",client.response);
		return -1;
	}
	if (! client.response->body) {
		client.response->headers = parse_headers(client.response->contents,&client.response->body);
		if (! client.response->headers) {
			error("No response headers on Response <%p>\n",client.response);
			return -1;
		}
	}
	client.response->done = client.response->body &&
		(len(client.response->contents) - client.response->body) >= inbound_content_length(client.response->contents,client.response->headers);
	if (client.response->done) dechunk_response();
	return 0;
}

int
begin_response()
{
	if (!client.response->headers) {
		error("Missing headers, adding new ones");
		client.response->headers = new_headers();
	}
	connection(client.response->headers,"close");
	transfer_encoding(client.response->headers,"chunked");
	server_name(client.response->headers,SERVER_VERSION);
	return client.response->contents || client.response->raw_contents;
}

int
end_response()
{
	write_chunk(client.response->socket,NULL,0);
	return 0;
}

