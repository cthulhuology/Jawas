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
#include "sockets.h"
#include "server.h"
#include "timers.h"

Client client;

void
disconnect()
{
	close_socket(client.socket);
	close(client.kq);
	exit(JAWAS_EXIT_DONE);
}

void
retry_request()
{
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
	if (process_request()) {
		retry_request();
		return;
	}
	client.request->retries = 0;
	if (!client.request->done) {
		add_read_socket(client.socket->fd);
		return;
	}
	client.socket->buf = NULL;
	client.response = new_response(client.request);
	parse_path(client.request);
	add_write_socket(client.response->socket->fd);
	return;
}


void
send_response()
{
	str host = parse_host(client.request,find_header(client.request->headers,_("Host")));
	str method = parse_method(client.request);
	begin_response();
	client.response->status = host && method ? dispatch_method(method) : error_handler(400);
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
client_poll(reg fd, enum event_types t) 
{
	switch (client.event) {
	case READ:
		closed_socket(client.socket,"Read failed") ? disconnect() : read_request();
		break;
	case WRITE:
		closed_socket(client.socket,"Write failed") ? disconnect() : send_response();
		break;
	case REQ:
		closed_socket(client.socket,"Request failed") ? disconnect() : write_request();
		break;
	case RESP:
		closed_socket(client.socket,"Response failed") ? disconnect() : read_response();
		break;	
	default:
		debug("UNKNOWN EVENT");
	}
}

void
run() 
{
	poll_events(client.kq, client_poll);
}

void
handle(reg fd)
{
	Socket s = accept_socket(fd,(server.http_sock == fd ? NULL : server.tls));
	nodelay(s);
	client.kq = kqueue();
	client.socket = s;
	client.request = open_request(client.socket);
	add_read_socket(client.socket->fd);
	timeout(IDLE_TIMEOUT,0);
	timer();
	while (!client.socket->closed && !client.alarm) run();
	exit(JAWAS_EXIT_DONE);
}
