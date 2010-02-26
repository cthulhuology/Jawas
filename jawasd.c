// jawas jawasd.c
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "daemon.h"
#include "server.h"

void
usage(char* name)
{
	fprintf(stderr,"Usage %s [-d] [port] [tls]\n", name);
	exit(1);
}

int
main(int argc, char** argv)
{
	int detach = (argc > 1 && !strncmp(argv[1],"-d",2)) ? 1 : 0;
	char* port = (argc > (1 + detach) ? argv[1 + detach] : SERVER_PORT);;
	char* tls_port = (argc > (2 + detach) ? argv[2 + detach] : TLS_SERVER_PORT);

	if (argc > 3) usage(argv[0]);

	demon(detach);	
	serve(atoi(port),atoi(tls_port));
	return 0;
}
