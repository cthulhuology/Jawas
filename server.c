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
#include "lua_db.h"

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
	if (server.procs >= MAX_PROCS) return;
	++server.procs;
	if (! (pid = fork())) return handle(fd);
	if (pid < 0) {
		perror("fork");
		exit(JAWAS_EXIT_FORK);
	}
	add_pid_monitor(pid);
}

void
wait_proc()
{
	int status;
	while (0 < waitpid(-1,&status,WNOHANG) && ! server.done) {
		--server.procs;
		if (status) fprintf(stderr,"Child exited with status %d\n",status);
	}
}

void
server_poll(reg fd, enum event_types t)
{
	wait_proc();
	external_port(fd) ? incoming(fd) :
		t != PROC ? reload(fd) : wait_proc();
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
	server.http_sock = open_socket(port);
	if (use_tls) {
		server.tls_sock = open_socket(tls_port);
	}
#ifdef LINUX
	server.kq = epoll_create1(0);
	server.in = inotify_init1(IN_NONBLOCK);
#else
	server.kq = kqueue();
	server.in = 0;
#endif
	monitor_socket(server.http_sock);
	if (use_tls) {
		monitor_socket(server.tls_sock);
	}
	general_signal_handlers();
	socket_signal_handlers();
restart:
	new_region();
	server.cwd = NULL;
	server.done = 0;
	server.restart = 0;
	if (use_tls) {
		server.tls = init_tls(TLS_KEYFILE,TLS_PASSWORD);
		server.tls_client = client_tls("certs");
	}
	init_strings();
	debug("Loading files...");
	load_files();
	debug("Done Loading files...");
	server.time = time(NULL);
	PrepareProcedures("public"); 
	while(!server.done) watch();
	// kill our process group
	kill(0,SIGTERM);
	if (server.restart) goto restart;	
	exit(JAWAS_EXIT_DONE);
}

