// Headers
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "pages.h"
#include "buffers.h"
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
	int i;
	if (! headers) return;
	for (i=0; i < MAX_HEADERS && headers[i].key; ++i) {
		free_buffer(headers[i].key);
		free_buffer(headers[i].value);
	}
	free_page((Page)headers);
}

Buffer
find_header(Headers headers, char* key)
{
	int i;
	if (! headers || ! key) return NULL;
	for (i = 0; i < MAX_HEADERS && headers[i].key; ++i ) {
		if (!strcasecmp(headers[i].key->data,key)) return headers[i].value;
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
append_header(Headers headers, Buffer key, Buffer value)
{
	int i;
	i = free_header_slot(headers);
	if (i > MAX_HEADERS) return NULL;
	headers[i].key = key;
	headers[i].value = value;
	return headers;
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

