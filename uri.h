// uri.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __URI_H__
#define __URI_H__

#include "str.h"
#include "requests.h"

str uri_encode(str s);
str uri_decode(str s);

Headers parse_uri_encoded(Headers head, Buffer buf, int pos, int len);

static char mark_chars[] = "-_.!~*'()";
static int mark_len = 9;
static char hex_chars[] = "0123456789abcdef";

#endif
