// jawas jawasd.c
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "log.h"
#include "server.h"

#include "lua_json.h"

int
main(int argc, char** argv)
{
	int murder = 0;
	int child = 0;
	int done = 0;
	int child_status = 0;
	int detach = 0;

	if (argc > 1 && !strncmp(argv[1],"-d",2)) {
		detach = 1;	
	}
	char* port = (argc > (1 + detach) ? argv[1 + detach] : SERVER_PORT);;
	char* tls_port = (argc > (2 + detach) ? argv[2 + detach] : TLS_SERVER_PORT);

	if (argc > 3) {
		fprintf(stderr,"Usage %s [-d] [port] [tls]\n", argv[0]);
		exit(1);
	}
	
restart:
	if (murder)
		kill(child,SIGTERM);
	if (done)
		exit(0);
	if (detach)
		child = fork();
	if (child == 0) {
		serve(atoi(port),atoi(tls_port));
		if (!srv) return 1;
		notice("[%i] Jawasd Running...",getpid());
		while (! srv->done) run();	
		stop();
	} else {
		char* pid = NULL;
		int pidfd = open("jawas.pid",O_CREAT|O_WRONLY|O_TRUNC,0600);
		if (pidfd > 0) {
			asprintf(&pid,"%i\n",getpid());
			write(pidfd,pid,strlen(pid));
			close(pidfd);
		} else {
			perror("open");
		}
		void kill_all(int sig) {
			murder = 1;
			done = 1;
		}
		void kill_child(int sig) {
			murder = 1;
		}
		signal(SIGQUIT,kill_all);
		signal(SIGTERM,kill_all);
		signal(SIGHUP,kill_child);
		while (!waitpid(child,&child_status,WNOHANG)) {
			if (murder)
				kill(child,SIGTERM);
			sleep(1);
		}
		goto restart;
	}
	return 0;
}
