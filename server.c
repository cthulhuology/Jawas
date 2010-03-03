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
	pid_t pid = 0;
	if (! (pid = fork())) handle(fd);
	if (pid < 0) {
		perror("fork");
		exit(1);
	}
	add_pid_monitor(pid);
}

void
wait_proc(reg pid)
{
	int status;
	if (0 > waitpid(pid,&status,WNOHANG)) {
		perror("waitpid");
		exit(1);
	}
}

void
server_poll(reg fd, enum event_types t)
{
	external_port(fd) ? incoming(fd) :
		t == PROC ? wait_proc(fd): reload(fd);
}

void
watch()
{
	poll_events(server.kq,server_poll);
}

void
stop()
{
	close(server.http_sock);
	close(server.tls_sock);
	close(server.kq);
	close_files();
}

void
serve(int port, int tls_port)
{
	new_region();
	server.cwd = NULL;
	server.done = 0;
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

