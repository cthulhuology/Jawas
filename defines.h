// defines.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#include "config.h"

#ifdef BITS64
#define reg uint64_t
#else
#define reg int
#endif

#define SERVER_VERSION "Jawas/1.0"

#define MAX_PROCS 200 

#define RUNTIME_SIZE 8L * 1024L * 1024L
#define CONTEXT_SIZE 8192
#define MAX_INDEX_LEN 12
#define MAX_WRITE_SIZE 1460
#define SOCKET_CONNECT_TIMEOUT 10
#define MAX_RETRIES 20

#define Max_Buffer_Size 4000

#define error(msg ...)	log_msg(0,msg)
#define dblog(msg ...) log_msg(1,msg)
#define notice(msg ...)	log_msg(2,msg)
#define debug(msg ...)	log_msg(3,msg)

#define JAWAS_EXIT_DONE 0
#define JAWAS_EXIT_USAGE 1
#define JAWAS_EXIT_FORK 2
#define JAWAS_EXIT_WAIT 3

