// signals.c
//
// Copyright (C) 2007 David J. Goehrig 
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "signals.h"
#include "client.h"
#include "server.h"

void
restart_signal_handler()
{
	server.done = 1;
	server.restart = 1;
}

void
gen_signal_handler()
{
	server.done = 1;
}

void
alrm_signal_handler()
{
	client.alarm = 1;
}

void
general_signal_handlers()
{
	signal(SIGHUP,restart_signal_handler);
	signal(SIGINT,gen_signal_handler);
	signal(SIGTERM,gen_signal_handler);
	signal(SIGALRM,alrm_signal_handler);
}
