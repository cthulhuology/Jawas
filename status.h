// Status.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __STATUS_H__
#define __STATUS_H__

#include "responses.h"
#include "server.h"

char* status_line(int code);
int error_handler(Server srv, int code, Response resp);

#endif
