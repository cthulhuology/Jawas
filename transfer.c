// transfer.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "transfer.h"

int
is_chunked(Headers headers)
{
	str enc = find_header(headers, $(Transfer_Encoding_MSG));
 	return enc && icmp(enc,$("chunked"));
}

int
chunked_length(str src)
{
	int total = 0;
	str line;
	int pos = skip_headers(src,0);
	do {
		line = read_line(src,pos);	
		pos += len(line) + 2;
		pos += str_hex(line);
		total += str_hex(line);
	} while (len(line) != 0);
	return total;
}

int
inbound_content_length(str src, Headers headers)
{
	if (!src || !headers) return 0;
	return is_chunked(headers) ?
		chunked_length(src) :
		str_int(find_header(headers, $("Content-Length")));
}

int
outbound_content_length(str src, File fc)
{
	return src ? len(src) : fc ? fc->st.st_size : 0;
}
