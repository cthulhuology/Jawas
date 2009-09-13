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
	str enc = find_header(headers, Str(Transfer_Encoding_MSG));
 	return enc && icmp(enc,Str("chunked"));
}

int
chunked_length(str src)
{
	int total = 0;
	int pos = 0;
	int delta = 0;
	str line;
	str t;

	pos = skip_headers(src,0);
	for (t = seek(src,pos); t; t = seek(src,pos)) {
		line = read_line(src,pos);	
		if (len(line) == 0) break;
		delta = str_hex(line);
		total += delta;
		pos += delta + len(line) + 4;
		if (delta == 0) 
			return total;
	}
	return 0x7fffffff;
}

int
inbound_content_length(str src, Headers headers)
{
	if (!src || !headers) return 0;
	return is_chunked(headers) ?
		chunked_length(src) :
		str_int(find_header(headers, Str("Content-Length")));
}

int
outbound_content_length(str src, File fc)
{
//	debug("outbound content lenght src is [%s]\n",src);
	return src ? len(src) : fc ? fc->st.st_size : 0;
}
