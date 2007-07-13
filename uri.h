// uri.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __URI_H__
#define __URI_H__

#include "requests.h"

char* file_path(Request req, char* filename, int flen);
char* request_path(Request req);

#endif
