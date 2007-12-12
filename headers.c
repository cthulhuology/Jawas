// Headers
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "str.h"
#include "headers.h"

Headers
new_headers()
{
	Headers retval;
	retval = (Headers)salloc(HEADERS_SIZE);
	memset(retval,0,HEADERS_SIZE);
	return retval;
}

str
find_header(Headers headers, char* key)
{
	int i;
	if (! headers || ! key) return NULL;
	int len = strlen(key);
	for (i = 0; i < headers->nslots && i < MAX_HEADERS; ++i ) {
		if (! headers->slots[i].key) continue;
		if (len == headers->slots[i].key->len 
		&& !strncasecmp(headers->slots[i].key->data,key,len)) 
			return headers->slots[i].value;
	}
	return NULL;
}

int
free_header_slot(Headers headers)
{
	if (headers->nslots < MAX_HEADERS) 
		return headers->nslots++;
	error("MAX HEADERS exceeded");
	halt;
}

Headers
append_header(Headers headers, str key, str value)
{
	int i;
	if (!headers)
		headers = new_headers();
	i = free_header_slot(headers);
	if (i > MAX_HEADERS) return NULL;
	headers->slots[i].key = key;
	headers->slots[i].value = value;
	debug("Append headers [%s = %s]",Key(headers,i),Value(headers,i));
	return headers;
}

Headers
sort_headers(Headers kv)
{
	int i, j;
	Headers retval = new_headers();
	retval->nslots = kv->nslots;
	over(kv,i) {
		retval->slots[i].key = Key(kv,i);
		retval->slots[i].value = Value(kv,i);
	}
	over(retval,i) {
		skip_null(retval,i);
		str pivot = Key(retval,i);
		overs(retval,j,i+1) {
			if (lesser_str(Key(retval,j),Key(retval,i))) {
				str tmp_key = Key(retval,i);
				str tmp_value =  Value(retval,i);
				retval->slots[i].key = Key(retval,j);
				retval->slots[i].value = Value(retval,j);
				retval->slots[j].key = tmp_key;
				retval->slots[j].value = tmp_value;
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
	over(kv,i) {
		skip_null(kv,i);
		debug("i is %i",i);
		debug("retval is %p",retval);
		retval = retval ? Str("%s, %s : %s", retval, kv->slots[i].key, kv->slots[i].value)
				: Str("%s : %s", kv->slots[i].key,kv->slots[i].value); 
	}
	return retval;
}

Buffer
print_headers(Buffer dst, Headers src)
{
	int i;
	Buffer retval = dst;
	over(src,i) {
		skip_null(src,i);
		retval = print_buffer(retval,"%s: %s\r\n",Key(src,i),Value(src,i));
	}
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

