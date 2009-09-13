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
#include "server.h"
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
		if (ncmp(Str("boundary="),t,9)) {
			t = from(enc, i + 9, el - (i+9));
			debug("Boundary is %s",t);
			return Str("--%s",t);
		}
	}
	return NULL;
}

str
parse_name(str src, int pos)
{
	str retval = NULL;
	int bl = len(src);
	int off = 6 + search(src,pos,Str("name=\""));
	debug("name offset %i, %i",off,bl);
	if (off >= bl) return NULL;
	debug("Working offset %i",off);
	int end = find(src,off+1,"\"");
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
is_file(str src, int pos, int end)
{
	int l = len(src);
	int off = 6 + search(src,pos,Str("filename=\""));
	return off < l && off < end;
}

int
skip_content_headers(str src, int pos)
{
	return 4 + search(src,pos,Str("\r\n\r\n"));
}

str
save_contents(str src, int pos, int end)
{
	int delta;
	str t = NULL;
	str filename = temp_file();
	debug("Save contents filename %s",filename);
	char* fname = dump(filename);
	int fd = open(fname,O_WRONLY|O_CREAT,0644);
	free(fname);
	if (fd < 0 ) {
		error("Failed to open file for writing %s",filename);
		perror("open");
		return filename;	
	}
//	debug("Writing [%i,%i)",pos,end);
	for (t = seek(src,pos); t && t->pos < end; t = seek(src,pos)) {
		delta = pos - t->pos;
		if (end <= t->length + t->pos)  {
//			debug("A. Writing chunk [%i,%i]",delta + t->pos, end);
			write(fd,&t->data[delta],end - t->pos - delta);
			break; // done
		} else {
//			debug("B. Writing chunk [%i,%i]",delta + t->pos, t->pos + t->length);
			write(fd,&t->data[delta],t->length - delta);
			pos = t->pos + t->length;
		}
	}
	close(fd);
	return filename;		
}

Headers
parse_multipart_body(Headers headers, str enctype)
{
	str dstname;
	int i, e, n, l = len(Req->contents);
	str boundary = find_boundary(enctype);
	debug("Boundary: %s", boundary);
	if (!boundary) {
		error("multipart/form-data without boundary");
		return headers;
	}
	int bl = len(boundary);
	for (i = search(Req->contents,Req->body,boundary); 
		i < l;
		i = n + bl) {
		n = search(Req->contents,i+1,boundary);
		debug("Content area [%s]",from(Req->contents,i,n-i));
		str srcname = parse_name(Req->contents,i);
		if (! srcname) {
			i = skip_content_headers(Req->contents,i);
			continue;
		}
		e = skip_content_headers(Req->contents,i);
		if (is_file(Req->contents,i,e)) {
			debug("%s is a file",srcname);
			dstname = save_contents(Req->contents,e,n-2);
		} else {
			debug("%s is a form value",srcname);
			dstname = from(Req->contents,e,n-2-e);
		}
		debug("[%s] = [%s]",srcname,dstname);
		headers = append_header(headers,srcname,dstname);
	}
	return headers;
}
