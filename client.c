// client.c
//
// © 2010 David J. Goehrig
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "log.h"
#include "client.h"
#include "methods.h"
#include "status.h"

Client client;

void
disconnect()
{
	debug("Disconnect %p, %i",client.socket,client.socket->fd);
	close_socket(client.socket);
	exit(0);
}

void
retry_request()
{
	debug("Retry Request %p, %i",client.request,client.request->retries);
	if (client.request->retries >= MAX_RETRIES)  {
		error("Failed to read request\n");
		disconnect();
		return;
	}
	add_read_socket(client.request->socket->fd);
}

void
read_request()
{
	debug("Read Request %p",client.request);
	if (process_request()) {
		retry_request();
		return;
	}
	client.request->retries = 0;
	if (!client.request->done) {
		add_read_socket(client.socket->fd);
		return;
	}
	debug("Read Request %p, %i done",client.request,client.socket->fd);
	client.socket->buf = NULL;
	client.response = new_response(client.request);
	parse_path(client.request);
	add_write_socket(client.response->socket->fd);
	return;
}


void
send_response()
{
	debug("Send response to %d",client.socket->fd);
	str host = parse_host(client.request,find_header(client.request->headers,_("Host")));
	debug("Connect to host %s",host);
	str method = parse_method(client.request);
	debug("Request method %s",method);
	begin_response();
	client.response->status = host && method ?
		dispatch_method(method) :
		error_handler(400);
	debug("Sent response with status %d", client.response->status);
	end_response();
	disconnect();
}

void
read_response()
{
	if (process_response()) {
		error("Failed to read response\n");
		disconnect();		
		return;
	}
	if (!client.response->done) {
		add_resp_socket(client.socket->fd);
		return;
	}
	Response tmp = client.response;
	client.request = client.request = client.response->request;
	client.socket = client.request->socket;
	client.response = client.request->response;
	append_header(client.response->headers,_("data"),from(tmp->contents,tmp->body,len(tmp->contents) - tmp->body));
	append_header(client.response->headers,_("status"),from(tmp->contents,9,3));
	close_socket(tmp->socket);
	connection(client.response->headers,"close");
	add_write_socket(client.response->socket->fd);
}

void
write_request()
{
	if (send_request(client.request)) {
		add_req_socket(client.request->socket->fd);
		return;
	}
	client.response = new_response(client.request);
	add_resp_socket(client.response->socket->fd);
}

void
run() 
{
	client.event = poll_events(client.kq,client.event);
	switch (client.event->type) {
	case READ:
		debug("Event READ");
		closed_socket(client.socket,"Read failed") ?
			disconnect():
			read_request();
		break;
	case WRITE:
		debug("Event WRITE");
		closed_socket(client.socket,"Write failed") ?
			disconnect() :
			send_response();
		break;
	case REQ:
		debug("Event REQ");
		closed_socket(client.socket,"Request failed") ?
			disconnect() :
			write_request();
			break;	
	case RESP:
		debug("Event RESP");
		closed_socket(client.socket,"Response failed") ?
			disconnect() :
			read_response();
		break;	
	default:
		debug("UNKNOWN EVENT");
	}
}

void
handle(Socket s)
{
	client.kq = kqueue();
	client.socket = s;
	client.request = open_request(client.socket);
	add_read_socket(client.socket->fd);
	while (!client.socket->closed) run();
	exit(0);
}

