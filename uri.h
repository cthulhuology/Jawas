// uri.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __URI_H__
#define __URI_H__

#include "requests.h"

extern int cwdlen;

char* file_path(char* host, int hlen, char* filename, int flen);
char* request_path(Request req);

char* uri_encode(char* str);
char* uri_decode(char* str);

static char mark_chars[] = "-_.!~*'()";
static int mark_len = 9;
static char hex_chars[] = "0123456789abcdef";

#endif
