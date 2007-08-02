// Headers
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HEADERS_H__
#define __HEADERS_H__

#include "str.h"

typedef struct headers_struct*  Headers;
struct headers_struct {
	str key;
	str value;
};

Headers new_headers();
void free_headers(Headers headers);

str find_header(Headers headers, char* key);

Headers append_header(Headers headers, str key, str value);
void dump_headers(Headers headers);

Headers cache_control(Headers headers, char* value);
Headers connection(Headers headers, char* value);
Headers date_field(Headers headers, char* value);
Headers transfer_encoding(Headers headers, char* value);
Headers content_length(Headers headers, char* value);
Headers content_type(Headers headers, char* value);
Headers expires(Headers headers, char* value);
Headers location(Headers headers, char* value);
Headers server(Headers headers, char* value);

static char* Cache_Control_MSG = "Cache-Control";
static char* Connection_MSG = "Connection";
static char* Date_MSG = "Date";
static char* Transfer_Encoding_MSG = "Transfer-Encoding";
static char* Content_Length_MSG = "Content-Length";
static char* Content_Type_MSG = "Content-Type";
static char* Expires_MSG = "Expires";
static char* Location_MSG = "Location";
static char* Server_MSG = "Server";

#define HEADER_FUNC(f,k) \
Headers \
f (Headers headers, char* value) {\
	int i = free_header_slot(headers);\
	 headers[i].key = char_str(k,0);\
	 headers[i].value = char_str(value,0);\
	 return headers;\
}

#define MAX_HEADERS (getpagesize() / sizeof(struct headers_struct))

#endif
