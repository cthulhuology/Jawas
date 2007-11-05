// Headers
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "pages.h"
#include "str.h"
#include "headers.h"

Headers
new_headers()
{
	Headers retval;
	retval = (Headers)new_page();
	memset(retval,0,getpagesize());
	return retval;
}

void
free_headers(Headers headers)
{
	if (! headers) return;
	free_page((Page)headers);
}

str
find_header(Headers headers, char* key)
{
	int i;
	if (! headers || ! key) return NULL;
	int len = strlen(key);
	for (i = 0; i < MAX_HEADERS && headers[i].key; ++i ) {
		if (len == headers[i].key->len 
		&& !strncasecmp(headers[i].key->data,key,len)) 
			return headers[i].value;
	}
	return NULL;
}

int
free_header_slot(Headers headers)
{
	int i;
	for (i = 0; i < MAX_HEADERS && headers[i].key; ++i);
	return i;
}

Headers
append_header(Headers headers, str key, str value)
{
	int i;
	if (!headers)
		headers = new_headers();
	i = free_header_slot(headers);
	if (i > MAX_HEADERS) return NULL;
	headers[i].key = key;
	headers[i].value = value;
//	debug("Appending headers: %s = %s",key,value);
	return headers;
}

Headers
sort_headers(Headers kv)
{
	int i, j;
	Headers retval = new_headers();
	for (i = 0; i < MAX_HEADERS && kv[i].key; ++i) {
		retval[i].key = kv[i].key;
		retval[i].value = kv[i].value;
	}
	for (i = 0; i < MAX_HEADERS && retval[i].key; ++i) {
		str pivot = retval[i].key;
		for (j = i+1; j < MAX_HEADERS && retval[j].key; ++j) {
			if (lesser_str(retval[j].key,retval[i].key)) {
				str tmp_key = retval[i].key;
				str tmp_value =  retval[i].value;
				retval[i].key = retval[j].key;
				retval[i].value = retval[j].value;
				retval[j].key = tmp_key;
				retval[j].value = tmp_value;
			}
		}
	}
	return retval;
}

str
list_headers(Headers kv)
{
	str retval = NULL;
	int i;
	for (i = 0; i < MAX_HEADERS && kv[i].key; ++i) {
		debug("i is %i",i);
		debug("retval is %p",retval);
		retval = retval ? Str("%s, %s : %s", retval, kv[i].key, kv[i].value)
				: Str("%s : %s", kv[i].key,kv[i].value); 
	}
	return retval;
}

Buffer
print_headers(Buffer dst, Headers src)
{
	int i;
	Buffer retval = dst;
	for (i = 0; i < MAX_HEADERS && src[i].key; ++i) 
		retval = print_buffer(retval,"%s: %s\r\n",src[i].key,src[i].value);
	return retval;
}

HEADER_FUNC(cache_control,Cache_Control_MSG)
HEADER_FUNC(connection,Connection_MSG)
HEADER_FUNC(date_field,Date_MSG)
HEADER_FUNC(transfer_encoding,Transfer_Encoding_MSG)
HEADER_FUNC(content_length,Content_Length_MSG)
HEADER_FUNC(content_type,Content_Type_MSG)
HEADER_FUNC(expires,Expires_MSG)
HEADER_FUNC(location,Location_MSG)
HEADER_FUNC(server,Server_MSG)

