// forms.c 
// 
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "log.h"
#include "headers.h"
#include "client.h"
#include "forms.h"

str 
find_boundary(str enc)
{
	int i;
	str t;
	int el = len(enc);
	for (i = 19; i < el; ++i) {
		t = from(enc, i, el - i);
		debug("TMP %s",t);
		if (ncmp(_("boundary="),t,9)) {
			t = from(enc, i + 9, el - (i+9));
			debug("Boundary is %s",t);
			return _("--%s",t);
		}
	}
	return NULL;
}

str
parse_name(str src, int pos)
{
	str retval = NULL;
	int bl = len(src);
	int off = 6 + search(src,pos,_("name=\""));
	debug("name offset %i, %i",off,bl);
	if (off >= bl) return NULL;
	debug("Working offset %i",off);
	int end = find(src,off+1,"\"",1);
	debug("End working offset %i",end);
	if (end - off > 4000) {
		error("Filename exceeds legit size");
		return NULL;	
	}
	debug("off %i len %i",off,end-off);
	retval =  from(src,off,end-off);
	debug("Found file name: %s",retval);
	return retval;
}

int
found_file(str src, int pos, int end)
{
	int l = len(src);
	int off = 6 + search(src,pos,_("filename=\""));
	return off < l && off < end;
}

int
skip_content_headers(str src, int pos)
{
	return 4 + search(src,pos,_("\r\n\r\n"));
}

str
save_contents(str src, int pos, int end)
{
	str filename = temp_file();
	debug("Save contents filename %s",filename);
	char* fname = filename->data;
	int fd = open(fname,O_WRONLY|O_CREAT,0644);
	if (fd < 0 ) {
		error("Failed to open file for writing %s",filename);
		perror("open");
		return filename;	
	}
	write(fd,&src->data[pos],end - pos);
	close(fd);
	return filename;		
}

Headers
parse_multipart_body(Headers headers, str enctype)
{
	str dstname;
	int i, e, n, l = len(client.request->contents);
	str boundary = find_boundary(enctype);
	debug("Boundary: %s", boundary);
	if (!boundary) {
		error("multipart/form-data without boundary");
		return headers;
	}
	int bl = len(boundary);
	for (i = search(client.request->contents,client.request->body,boundary); 
		i < l;
		i = n + bl) {
		n = search(client.request->contents,i+1,boundary);
		debug("Content area [%s]",from(client.request->contents,i,n-i));
		str srcname = parse_name(client.request->contents,i);
		if (! srcname) {
			i = skip_content_headers(client.request->contents,i);
			continue;
		}
		e = skip_content_headers(client.request->contents,i);
		if (found_file(client.request->contents,i,e)) {
			debug("%s is a file",srcname);
			dstname = save_contents(client.request->contents,e,n-2);
		} else {
			debug("%s is a form value",srcname);
			dstname = from(client.request->contents,e,n-2-e);
		}
		debug("[%s] = [%s]",srcname,dstname);
		headers = append_header(headers,srcname,dstname);
	}
	return headers;
}
