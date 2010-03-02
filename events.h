// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reservered
//

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "defines.h"
#include "memory.h"
#include "sockets.h"
#include "requests.h"
#include "responses.h"

enum event_types { READ, WRITE, RESP, REQ, NODE, PROC };
enum event_flags { NONE, ONESHOT, SEOF, EXIT };

typedef void (*function)(reg fd, enum event_types t);

reg add_event(reg q, reg fd, enum event_types t, enum event_flags f);
void poll_events(reg q, function f);

void monitor_socket(reg f);
void add_read_socket(reg f);
void add_write_socket(reg f);
void add_req_socket(reg f);
void add_resp_socket(reg f);
void add_file_monitor(reg f);
void add_pid_monitor(pid_t p);

#endif
