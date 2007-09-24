// forms.c 
// 
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "headers.h"
#include "buffers.h"
#include "str.h"
#include "server.h"
#include "forms.h"

str 
find_boundary(str enc)
{
	int i;
	cstr tmp;
	debug("Enc data: %p len: %i",enc->data, enc->len);
	for (i = 19; i < enc->len; ++i) {
		tmp = Cstr(enc->data + i, enc->len - i);
		debug("TMP %x (%p, %i)",tmp, tmp->data, tmp->len);
		if (ncmp_str(Str("boundary="),tmp,9)) {
			tmp = Cstr(enc->data + i + 9, enc->len - (i+9));
			debug("tmp is %x (%p,%i)", tmp, tmp->data,tmp->len);
			return Str("--%x",tmp);
		}
	}
	return NULL;
}

str
parse_filename(Buffer buf, int pos)
{
	str retval = NULL;
	int len = length_buffer(buf);
	int off = 10 + search_buffer(buf,pos,Str("filename=\""),0);
	debug("filename offset %i, %i", off,len);
	if (off >= len) {
		off = 6 + search_buffer(buf,pos,Str("name=\""),0);
		debug("name offset %i, %i",off,len);
		if (off >= len) return NULL;
	}
	debug("Working offset %i",off);
	int end = find_buffer(buf,off+1,"\"");
	debug("End working offset %i",end);
	if (end - off > 4000) {
		error("Filename exceeds legit size");
		return NULL;	
	}
	debug("off %i len %i",off,end-off);
	retval =  read_str(buf,off,end-off);
	debug("Found file name: %s",retval);
	return retval;
}

int
skip_content_headers(Buffer buf, int pos)
{
	return 4 + search_buffer(buf,pos,Str("\r\n\r\n"),0);
}

str
save_contents(str fname, Buffer buf, int pos, int end)
{
	int delta;
	Buffer tmp = NULL;
	str filename = Str("/tmp/%i%i-%s",random(),time(NULL),fname);
	debug("Save contents filename %s",filename);
	int fd = open(filename->data,O_WRONLY|O_CREAT,0644);
	if (fd < 0 ) {
		error("Failed to open file for writing %s",filename);
		perror("open");
		return filename;	
	}
	debug("Writing [%i,%i)",pos,end);
	for (tmp = seek_buffer(buf,pos); tmp && tmp->pos < end; tmp = seek_buffer(buf,pos)) {
		delta = pos - tmp->pos;
		if (end <= tmp->length + tmp->pos)  {
			debug("A. Writing chunk [%i,%i]",delta + tmp->pos, end);
			write(fd,&tmp->data[delta],end - tmp->pos - delta);
			break; // done
		} else {
			debug("B. Writing chunk [%i,%i]",delta + tmp->pos, tmp->pos + tmp->length);
			write(fd,&tmp->data[delta],tmp->length - delta);
			pos = tmp->pos + tmp->length;
		}
	}
	close(fd);
	return filename;		
}

Headers
parse_multipart_body(Headers headers, str enctype)
{
	int i, n, len = length_buffer(Req->contents);
	str boundary = find_boundary(enctype);
	debug("Boundary: %s", boundary);
	if (!boundary) {
		error("multipart/form-data without boundary");
		return headers;
	}
	for (i = search_buffer(Req->contents,Req->body,boundary,0); 
		i < len;
		i = n + boundary->len + 2) {
		debug("Search buffer start offset %i",i);
		n = search_buffer(Req->contents,Req->body + i + boundary->len,boundary,0);
		debug("Search buffer end offset %i",n);
		str srcname = parse_filename(Req->contents,i);
		if (! srcname) {
			i = skip_content_headers(Req->contents,i);
			continue;
		}
		debug("srcname = %s",srcname);
		i = skip_content_headers(Req->contents,i);
		debug("Offset of contents is %i",i);
		str dstname = save_contents(srcname,Req->contents,i,n-2);
		debug("dstname = %s",dstname);
		headers = append_header(headers,srcname,dstname);
	}
	return headers;
}
