// uri.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __URI_H__
#define __URI_H__

#include "str.h"
#include "requests.h"

extern str cwd;

str file_path(str host, str filename);

char* uri_encode(char* str);
char* uri_decode(char* str);

str parse_method(Request req);
str parse_host(Request req);
str parse_path(Request req);

Headers parse_uri_encoded(Headers head, Buffer buf, int pos, int len);
Headers parse_query_string(Buffer buf);
Headers parse_post_request(Request req);

static char mark_chars[] = "-_.!~*'()";
static int mark_len = 9;
static char hex_chars[] = "0123456789abcdef";


#endif
