// Headers
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "str.h"
#include "log.h"
#include "headers.h"
#include "sockets.h"

Headers
new_headers()
{
	Headers retval = (Headers)reserve(HEADERS_SIZE);
	memset(retval,0,HEADERS_SIZE);
	return retval;
}

str
Key(Headers header, int slot)
{
	return 0 <= slot && slot < header->nslots ? header->slots[slot].key : NULL;
}

str
Value(Headers header, int slot)
{
	return 0 <= slot && slot < header->nslots ? header->slots[slot].value : NULL;
}

str
find_header(Headers headers, str key)
{
	int i;
	if (! headers || ! key) return NULL;
	for (i = 0; i < headers->nslots && i < MAX_HEADERS; ++i ) {
		if (! headers->slots[i].key) continue;
		if (cmp(headers->slots[i].key,key)) return headers->slots[i].value;
	}
	return NULL;
}

int
free_header_slot(Headers headers)
{
	if (headers->nslots < MAX_HEADERS) return headers->nslots++;
	error("MAX HEADERS exceeded");
	halt;
	return 0; // never reach here
}

Headers
append_header(Headers headers, str key, str value)
{
	int i;
	if (!headers) headers = new_headers();
	i = free_header_slot(headers);
	if (i > MAX_HEADERS) return NULL;
	headers->slots[i].key = key;
	headers->slots[i].value = value;
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
		overs(retval,j,i+1) {
			if (lesser(Key(retval,j),Key(retval,i))) {
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
		retval = retval ? _("%s, %s : %s", retval, kv->slots[i].key, kv->slots[i].value)
				: _("%s : %s", kv->slots[i].key,kv->slots[i].value); 
	}
	return retval;
}

str
print_headers(str dst, Headers src)
{
	int i;
	str retval = dst;
	over(src,i) {
		skip_null(src,i);
		retval = retval ? _("%s%s: %s\r\n",retval, Key(src,i),Value(src,i)):
			_("%s: %s\r\n",Key(src,i),Value(src,i));
	}
	return retval;
}

str
url_encode_headers(Headers src)
{
	int i;
	if (! src) return NULL;
	str retval = _("%s=%s",Key(src,0),Value(src,0));
	overs(src,i,1) {
		skip_null(src,i);
		retval = _("%s&%s=%s",retval,Key(src,i),Value(src,i));
	}
	return retval;
}

Headers
parse_headers(str buf, int* body)
{
	char c;
	int i,l,o,reset = 0,count;
	int lb = len(buf);
	Headers headers = new_headers();
	if (! headers) {
		error("Failed to allocate new headers\n");
		return NULL;
	}
	count = 0;
	for (o = 0; o < lb; ++o ) {
		c = at(buf,o);
		if (c == '\r' || c == '\n') break;
	}
	if (o >= lb) {
		error("No line breaks found! Bad request\n");
		return NULL;
	}
	for (i = 0; i < MAX_HEADERS && o < lb; ++o) {
		c = at(buf,o);
		if (c == '\r' || c == '\n') {
			reset = 1;
			++count;
			if (count > 2) {
				*body = o+1;
				return headers;
			}
			continue;
		}
		count = 0;
		if (reset && ! Key(headers,i)) {
			for (l = 1; (o + l) < lb && at(buf,o+l) != ':'; ++l);
			headers->nslots++;
			headers->slots[i].key = ref(buf->data+o,l);
			o += l-1;
			c = at(buf,o);
		}
		if (reset && c == ':') {
			o += 1;
			while(isspace(c = at(buf,o))) ++o;
			for (l = 1; (o + l) < lb && c != '\r' && c != '\n'; ++l) c = at(buf,o+l); 
			headers->slots[i].value = ref(buf->data+o,l-1);
			debug("Headers[%i] [%s] = [%s]",i,Key(headers,i),Value(headers,i));
			reset = 0;
			o += l-1;
			++i;
		}
	}
	debug("***** BODY NOT SET!!!");
	return headers;
}

int
send_headers(Socket sc, Headers headers)
{
	int i;
	int total = 0;
	if (!headers) return total;
	over(headers,i) {
		skip_null(headers,i);
		debug("[SEND HEADERS] [%s] [%s]",Key(headers,i),Value(headers,i));
		total += write_socket(sc,Key(headers,i));
		total += write_to_socket(sc,": ",2);
		total += write_socket(sc,Value(headers,i));
		total += write_to_socket(sc,"\r\n",2);
	}
	total += write_to_socket(sc,"\r\n",2);
	return total;
}

HEADER_FUNC(cache_control,Cache_Control_MSG)
HEADER_FUNC(connection,Connection_MSG)
HEADER_FUNC(date_field,Date_MSG)
HEADER_FUNC(transfer_encoding,Transfer_Encoding_MSG)
HEADER_FUNC(content_length,Content_Length_MSG)
HEADER_FUNC(content_type,Content_Type_MSG)
HEADER_FUNC(expires,Expires_MSG)
HEADER_FUNC(location,Location_MSG)
HEADER_FUNC(server_name,Server_MSG)

