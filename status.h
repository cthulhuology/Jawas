// Status.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __STATUS_H__
#define __STATUS_H__

#include "responses.h"
#include "server.h"

struct status_struct {
	int code;
	int len;
	char* reason;
	char* filename;
};

str status_line(int code);
int error_handler(int code);

#endif
