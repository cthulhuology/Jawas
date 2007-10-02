// jawas jawasd.c
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "log.h"
#include "server.h"

int
main(int argc, char** argv)
{
	int child = 0;
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
	if (detach) {
		child = fork();
	}
	if (child == 0) {
		serve(atoi(port),atoi(tls_port));
		if (!srv) return 1;
		notice("[%i] Jawasd Running...",getpid());
		while (! srv->done) run();	
		stop();
	} else {
		waitpid(child,&child_status,0);
		goto restart;
	}
	return 0;
}

