// defines.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#include "config.h"

#define SERVER_VERSION "Jawas/1.0"

#define min(x,y) (x < y ? x : y)
#define max(x,y) (x > y ? x : y)
#define between(a,x,b)	(x >= a && x <= b)

#define RUNTIME_SIZE 8L * 1024L * 1024L
#define CONTEXT_SIZE 8192
#define MAX_INDEX_LEN 12
#define MAX_WRITE_SIZE (getpagesize() * 10)

#define error(msg ...)	log_msg(0,msg)
#define notice(msg ...)	log_msg(1,msg)
#define debug(msg ...)	log_msg(2,msg)

#define Sock	srv->sock
#define Req	srv->req
#define Resp	srv->resp

