// Status.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __STATUS_H__
#define __STATUS_H__

#include "sockets.h"

struct status_struct {
	int code;
	int len;
	char* reason;
	char* filename;
};

str status_line(int code);
int error_handler(int code);
int send_status(Socket sc, int code);

#endif
