// jawas server.c
// Copyright (C) 2007,2010 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "client.h"
#include "server.h"
#include "signals.h"
#include "log.h"
#include "files.h"
#include "lws.h"

Server server;

reg
external_port(reg fd)
{
	return fd == server.http_sock || fd == server.tls_sock;
}

void
incoming(reg fd)
{
	debug("Incoming %i",fd);
	if (!fd) return;
	Socket s = accept_socket(fd,(server.http_sock == fd ? NULL : server.tls));
	if (! fork()) handle(s);
}

void
watch()
{
	Event e = poll_events(server.kq,server.event);
	for (server.event = NULL; e; e = e->next) {
		if (!e->fd) continue;
		if (external_port(e->fd)) incoming(e->fd);
		if (e->type == NODE) reload(e->fd);
	}
}

void
stop()
{
	close(server.http_sock);
	close(server.tls_sock);
	close(server.kq);
	close_sockets();
	close_files();
}

void
serve(int port, int tls_port)
{
	init_regions();
	server.cwd = NULL;
	server.done = 0;
	server.event = NULL;
	server.kq = kqueue();
	server.http_sock = open_socket(port);
	server.tls_sock = open_socket(tls_port);
	server.tls = init_tls(TLS_KEYFILE,TLS_PASSWORD);
	server.tls_client = client_tls("certs");
	monitor_socket(server.http_sock);
	monitor_socket(server.tls_sock);
	general_signal_handlers();
	socket_signal_handlers();
	init_strings();
	load_files();
	server.time = time(NULL);
	while(!server.done) watch();
	exit(0);
}

