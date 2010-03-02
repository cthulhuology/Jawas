// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "defines.h"
#include "include.h"
#include "memory.h"
#include "log.h"
#include "defines.h"
#include "client.h"
#include "server.h"
#include "events.h"

void 
monitor_socket(reg f) 
{
	add_event(server.kq, f, READ, NONE); 
}

void
add_read_socket(reg f)
{
	client.event = READ;
	add_event(client.kq, f, READ, ONESHOT);
}

void
add_write_socket(reg f)
{
	client.event = WRITE;
	add_event(client.kq, f, WRITE, ONESHOT);
}

void
add_req_socket(reg f)
{
	client.event = REQ;
	add_event(client.kq, f, REQ, ONESHOT);
}

void
add_resp_socket(reg f)
{
	client.event = RESP;
	add_event(client.kq, f, RESP, ONESHOT);
}

void
add_file_monitor(reg f)
{
	add_event(server.kq, f, NODE, ONESHOT);
}

void
add_pid_monitor(pid_t p)
{
	add_event(server.kq, p, PROC, EXIT);
}
