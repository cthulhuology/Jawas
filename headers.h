// Headers
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HEADERS_H__
#define __HEADERS_H__

#include "str.h"
#include "buffers.h"

#define MAX_HEADERS 250
#define HEADERS_SIZE sizeof(struct headers_struct)
#define over(x,y) for(y=0; y < x->nslots && y < MAX_HEADERS; ++y) 
#define overs(x,y,z) for(y=z; y < x->nslots && y < MAX_HEADERS; ++y) 
#define skip_null(x,y) if (x->slots[y].key == NULL) continue;

#define Key(o,s) (0 <= s && s < o->nslots ? o->slots[s].key : NULL)
#define Value(o,s) (0 <= s && s < o->nslots ? o->slots[s].value : NULL)

typedef struct headers_struct*  Headers;
struct headers_data_struct {
	str key;
	str value;
};
struct headers_struct {
	int nslots;
	struct headers_data_struct slots[MAX_HEADERS];
};

Headers new_headers();

str find_header(Headers headers, char* key);
str list_headers(Headers kv);
Buffer print_headers(Buffer dst, Headers src);

Headers append_header(Headers headers, str key, str value);
void dump_headers(Headers headers);

Headers sort_headers(Headers kv);

Headers cache_control(Headers headers, const char* value);
Headers connection(Headers headers, const char* value);
Headers date_field(Headers headers, const char* value);
Headers transfer_encoding(Headers headers, const char* value);
Headers content_length(Headers headers, const char* value);
Headers content_type(Headers headers, const char* value);
Headers expires(Headers headers, const char* value);
Headers location(Headers headers, const char* value);
Headers server(Headers headers, const char* value);

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
f (Headers headers, const char* value) {\
	int i = free_header_slot(headers);\
	 headers->slots[i].key = char_str(k,0);\
	 headers->slots[i].value = char_str(value,0);\
	 return headers;\
}


#endif
