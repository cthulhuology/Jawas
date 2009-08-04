// Headers
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HEADERS_H__
#define __HEADERS_H__

#include "sockets.h"
#include "str.h"
#include "strings.h"

#define MAX_HEADERS 250
#define HEADERS_SIZE sizeof(struct headers_struct)
#define over(x,y) for(y=0; y < x->nslots && y < MAX_HEADERS; ++y) 
#define overs(x,y,z) for(y=z; y < x->nslots && y < MAX_HEADERS; ++y) 
#define skip_null(x,y) if (x->slots[y].key == NULL) continue;

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
Headers parse_headers(str buf, int* body);
Headers append_header(Headers headers, str key, str value);

str Key(Headers headers, int slot);
str Value(Headers headers, int slot);

str find_header(Headers headers, str key);
str list_headers(Headers kv);

void dump_headers(Headers headers);
str print_headers(str dst, Headers src);
str url_encode_headers(Headers src);
int send_headers(Socket sc, Headers headers);

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


#define HEADER_FUNC(f,k) \
Headers \
f (Headers headers, const char* value) {\
	int i = free_header_slot(headers);\
	 headers->slots[i].key = copy(k,0);\
	 headers->slots[i].value = copy(value,0);\
	 return headers;\
}


#endif
