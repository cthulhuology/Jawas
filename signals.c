// signals.c
//
// Copyright (C) 2007 David J. Goehrig 
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "signals.h"
#include "server.h"

void
gen_signal_handler()
{
	srv->done = 1;
}

void
general_signal_handlers()
{
	signal(SIGHUP,gen_signal_handler);
	signal(SIGINT,gen_signal_handler);
	signal(SIGTERM,gen_signal_handler);
}
