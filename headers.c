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
#include "sockets.h"

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

Headers
parse_headers(Buffer buf, int* body)
{
	char c;
	int i,o,l,reset,count;
	int len = length_buffer(buf);
	Headers headers = new_headers();
	if (! headers) {
		error("Failed to allocate new headers\n");
		return NULL;
	}
	count = 0;
	for (o = 0; o < len; ++o ) {
		c = fetch_buffer(buf,o);
		if (c == '\r' || c == '\n') break;
	}
	if (o >= len) {
		error("No line breaks found! Bad request\n");
		return NULL;
	}
	for (i = 0; i < MAX_HEADERS && o < len; ++o) {
		c = fetch_buffer(buf,o);
		if (c == '\r' || c == '\n') {
			reset = 1;
			++count;
			if (count > 2) {
				*body = o+1;
				//debug("=== BODY ===");
				//dump_buffer(buf,*body);
				//debug("=== DONE ===");
				return headers;
			}
			continue;
		}
		count = 0;
		if (reset && ! Key(headers,i)) {
			for (l = 1; (o + l) < len && fetch_buffer(buf,o+l) != ':'; ++l);
			headers->nslots++;
			headers->slots[i].key = read_str(buf,o,l);
			o += l-1;
			c = fetch_buffer(buf,o);
		}
		if (reset && c == ':') {
			o += 1;
			while(isspace(c = fetch_buffer(buf,o))) ++o;
			for (l = 1; (o + l) < len && c != '\r' && c != '\n'; ++l) c = fetch_buffer(buf,o+l); 
			headers->slots[i].value = read_str(buf,o,l-1);
			debug("Headers[%i] [%s=%s]",i,Key(headers,i),Value(headers,i));
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
		str key = Key(headers,i);
		str value = Value(headers,i);
		if (key && value) {
			total += write_socket(sc,key->data,key->len);
			total += write_socket(sc,": ",2);
			total += write_socket(sc,value->data,value->len);
			total += write_socket(sc,"\r\n",2);
		}
	}
	total += write_socket(sc,"\r\n",2);
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
HEADER_FUNC(server,Server_MSG)

